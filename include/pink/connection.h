#ifndef PINK_CONNECTION_H_
#define PINK_CONNECTION_H_

#include <memory>

namespace pink {

class Dispatcher;
class IOThread;

class Connection {
 public:
  struct IoHandler;

  Connection();
  virtual ~Connection() {}
  bool InitConnection(int fd, std::shared_ptr<IOThread> io_thread,
                      Dispatcher* dispacher);

  int fd() { return fd_; }

  bool Connect() { return true; } // For client

  void PerformRead();
  void PerformWrite();
  int Write(const char* msg, size_t size);

  virtual bool OnDataAvailable(size_t size) = 0;

  void Close();

 protected:
  char read_buf_[4096];
  size_t buf_pos_;

 private:
  friend class Dispatcher;

  int fd_;
  std::shared_ptr<IOThread> io_thread_;
  std::shared_ptr<IoHandler> io_handler_;
  // Endpoint remote_side;
  // Endpoint local_side;

  Dispatcher* dispatcher_;
};

class ConnectionFactory {
 public:
  virtual ~ConnectionFactory() {}
  virtual std::shared_ptr<Connection> NewConnection() = 0;
};

}  // namespace pink

#endif  // PINK_CONNECTION_H_
