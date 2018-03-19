#pragma once

#include <memory>
#include <list>

namespace pink {

class Dispatcher;
class IOThread;

class Connection {
 public:
  struct IOHandler;

  Connection();
  virtual ~Connection() {}
  bool InitConnection(int fd, std::shared_ptr<IOThread> io_thread,
                      Dispatcher* dispacher);

  int fd() { return fd_; }

  bool Connect() { return true; } // For client

  bool Write(const char* msg, size_t size);

  virtual void GetReadBuffer(void** buffer, size_t* len) = 0;
  virtual bool OnDataAvailable(size_t size) = 0;

  void Close();

 private:
  friend class Dispatcher;

  void PerformRead();
  void PerformWrite();

  ssize_t WriteImpl(const char* msg, size_t size);

  int fd_;
  std::shared_ptr<IOThread> io_thread_;
  std::shared_ptr<IOHandler> io_handler_;
  // Endpoint remote_side;
  // Endpoint local_side;

  std::list<std::string> pending_output_; // TODO thread safe

  Dispatcher* dispatcher_;
};

class ConnectionFactory {
 public:
  virtual ~ConnectionFactory() {}
  virtual std::shared_ptr<Connection> NewConnection() = 0;
};

}  // namespace pink
