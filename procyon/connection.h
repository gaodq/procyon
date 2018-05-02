#pragma once

#include <memory>
#include <list>
#include <future>
#include <chrono>

#include "procyon/options.h"
#include "procyon/endpoint.h"
#include "procyon/io_thread.h"

namespace procyon {

class Dispatcher;
class Connection;

typedef std::shared_ptr<Connection> ConnectionPtr;

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  enum State {
    kNoConnect,
    kConnected,
  };

  struct IOHandler;

  Connection();
  virtual ~Connection() { Close(); }

  void InitConn(int conn_fd, std::shared_ptr<IOThread> io_thread,
                Dispatcher* dispacher, const EndPoint* remote_side,
                const EndPoint* local_side);

  int fd() const { return conn_fd_; }
  State state() { return state_; }
  ConnectionPtr getptr() { return shared_from_this(); }

  std::shared_ptr<EventbaseLoop> event_loop() {
    return io_thread_->event_loop();
  }

  virtual void GetReadBuffer(void** buffer, size_t* len) = 0;
  virtual void OnDataAvailable(size_t size) = 0;

  std::future<bool> Write(const void* data, size_t size);

  /* TODO Support Client
  bool Connect(const ClientOptions& opts,
               const EndPoint* remote_side,
               const EndPoint* local_side = nullptr);
  */

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

  int conn_fd_;
  std::chrono::time_point<std::chrono::system_clock> last_active_time_;
  std::shared_ptr<IOThread> io_thread_;
  std::shared_ptr<IOHandler> io_handler_;

  struct WriteRequest {
    std::string data;
    std::promise<bool> res;
  };
  std::mutex pending_output_mu_;
  std::list<WriteRequest> pending_output_; // TODO lock free

  Dispatcher* dispatcher_;
};

class ConnectionFactory {
 public:
  virtual ~ConnectionFactory() {}
  virtual std::shared_ptr<Connection> NewConnection() = 0;
};

}  // namespace procyon
