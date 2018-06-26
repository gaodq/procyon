#pragma once

#include <memory>
#include <list>
#include <future>
#include <chrono>

#include "procyon/endpoint.h"
#include "procyon/io_thread.h"

namespace procyon {

class Dispatcher;
class Connection;

typedef std::shared_ptr<Connection> ConnectionPtr;

struct ConnectionManager {
  virtual ~ConnectionManager() = default;

  virtual void OnConnClosed(int conn_fd) = 0;
  virtual void OnConnError(int conn_fd) = 0;
};

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  enum State {
    kNoConnect,
    kConnected,
  };

  struct IOHandler;

  Connection();
  virtual ~Connection() { CloseImpl(); }

  void InitConn(
    int conn_fd,
    std::shared_ptr<IOThread> io_thread,
    ConnectionManager* manager,
    const EndPoint& remote_side,
    const EndPoint& local_side);

  int fd() const { return conn_fd_; }
  State state() { return state_; }
  EndPoint local_side() { return local_side_; }
  EndPoint remote_side() { return remote_side_; }
  ConnectionPtr getptr() { return shared_from_this(); }

  virtual void GetReadBuffer(void** buffer, size_t* len) = 0;
  virtual void OnDataAvailable(size_t size) = 0;

  std::future<bool> Write(const void* data, size_t size);

  int IdleSeconds();

  void Close();

 private:
  friend class Dispatcher;

  State state_;

  EndPoint remote_side_;
  EndPoint local_side_;

  void PerformRead();
  void PerformWrite();

  ssize_t WriteImpl(const char* data, size_t size);
  void CloseImpl();

  int conn_fd_;
  std::chrono::time_point<std::chrono::system_clock> last_active_time_;
  std::shared_ptr<EventbaseLoop> event_loop_;
  std::shared_ptr<IOHandler> io_handler_;

  struct WriteRequest {
    std::string data;
    std::promise<bool> res;
  };
  std::mutex pending_output_mu_;
  std::list<WriteRequest> pending_output_; // TODO lock free

  ConnectionManager* conn_manager_;
};

class ConnectionFactory {
 public:
  virtual ~ConnectionFactory() {}
  virtual ConnectionPtr NewConnection() = 0;
};

}  // namespace procyon
