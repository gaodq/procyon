#include "pink/server.h"

#include <unistd.h>
#include <iostream>

#include "pink/linebased_conn.h"
#include "src/dispatcher.h"
#include "src/io_thread.h"
#include "src/eventbase_loop.h"

namespace pink {

int Server::Bind(const std::string& ip, int port) {
  return 0;
}

class CustomHandler : public LineBasedConn::LineMsgHandler {
 public:
  void HandleMessage(Connection* conn, const std::string& msg) override {
    std::cout << "receive: " << msg << std::endl;
    Write(conn, msg + "\r\n");
  }
};

class MyConnFactory : public ConnectionFactory {
 public:
  std::shared_ptr<Connection> NewConnection() override {
    return std::make_shared<LineBasedConn>(new CustomHandler);
  }
};

int Server::StartUntilInterrupt() {
  std::shared_ptr<IOThread> accept_thread(new IOThread);
  std::shared_ptr<IOThreadPool> worker_threads(new IOThreadPool(4));

  ServerOptions opts;
  opts.listen_ip = "0.0.0.0";
  opts.port = 8090;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.accept_thread = accept_thread;
  opts.worker_threads = worker_threads;

  std::shared_ptr<Dispatcher> dispatcher(new Dispatcher(opts));

  dispatcher->Bind();

  worker_threads->Start();
  accept_thread->Start();

  for (;;) {
    sleep(1);
  }
  return 0;
}

int Server::Interrupt() {
  return 0;
}

}  // namespace pink
