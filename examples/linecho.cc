#include "procyon/server.h"
#include "procyon/connection.h"
#include "procyon/linebased_proto.h"
#include "procyon/io_thread.h"

#include <unistd.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

class CustomHandler : public procyon::LineMsgHandler {
 public:
  void HandleNewLine(procyon::ConnectionPtr conn,
                     std::unique_ptr<procyon::IOBuf>&& line) override {
    std::string msg = line->ToString();
    std::cout << "Server receive: " << msg << std::endl;
    msg.append("\r\n");
    WriteLine(conn, msg);
  }
};

class MyConnFactory : public procyon::ConnectionFactory {
 public:
  std::shared_ptr<procyon::Connection> NewConnection() override {
    return std::make_shared<procyon::LineBasedConn>(new CustomHandler);
  }
};

int main() {
  procyon::ServerOptions opts;
  opts.port = 8088;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.worker_threads = std::make_shared<procyon::IOThreadPool>(24);

  procyon::Server server(opts);
  bool res = server.Start();
  if (!res) {
    return -1;
  }

  while (true) {
    sleep(1);
  }

  return 0;
}
