#pragma once

#include <memory>
#include <list>

#include "procyon/options.h"
#include "procyon/endpoint.h"
#include "procyon/io_thread.h"

namespace procyon {

class Dispatcher;

class Connection {
 public:
  enum State {
    kNoConnect,
    kConnecting,
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

  std::shared_ptr<EventbaseLoop> event_loop() {
    return io_thread_->event_loop();
  }

  virtual void GetReadBuffer(void** buffer, size_t* len) = 0;
  virtual void OnDataAvailable(size_t size) = 0;
  bool Write(const void* data, size_t size, bool block = false);

  bool Connect(const ClientOptions& opts,
               const EndPoint* remote_side,
               const EndPoint* local_side = nullptr);
  bool BlockRead(void* data, size_t size, size_t* received);

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
  uint64_t last_active_time_;
  std::shared_ptr<IOThread> io_thread_;
  std::shared_ptr<IOHandler> io_handler_;

  std::list<std::string> pending_output_; // TODO thread safe

  Dispatcher* dispatcher_;
};

class ConnectionFactory {
 public:
  virtual ~ConnectionFactory() {}
  virtual std::shared_ptr<Connection> NewConnection() = 0;
};

}  // namespace procyon
