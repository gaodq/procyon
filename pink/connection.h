#pragma once

#include <memory>
#include <list>

#include "pink/options.h"
#include "pink/endpoint.h"
#include "pink/io_thread.h"

namespace pink {

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

  virtual void GetReadBuffer(void** buffer, size_t* len) = 0;
  virtual bool OnDataAvailable(size_t size) = 0;
  bool Write(const void* data, size_t size, bool block = false);

  bool Connect(const ClientOptions& opts,
               const EndPoint* remote_side,
               const EndPoint* local_side = nullptr);
  bool BlockRead(void* data, size_t size, size_t* received);

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

}  // namespace pink
