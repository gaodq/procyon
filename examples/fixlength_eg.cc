#include "procyon/server.h"
#include "procyon/options.h"
#include "procyon/fixlength_proto.h"
#include "procyon/io_thread.h"

#include <google/protobuf/message.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

class ProtobufMsgHandler : public procyon::HTTPMsgHandler {
 public:
  virtual void HandleNewMsg(
      procyon::ConnectionPtr conn, const std::string& msg) override {
  }
};

class MyConnFactory : public procyon::ConnectionFactory {
 public:
  std::shared_ptr<procyon::Connection> NewConnection() override {
    return std::make_shared<procyon::FixlengthConn>(new ProtobufMsgHandler);
  }
};

int main() {
  procyon::ServerOptions opts;
  opts.port = 8099;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.worker_threads = std::make_shared<procyon::IOThreadPool>(1);

  worker_threads.Start();

  procyon::Server server(opts);
  bool res = server.Start();
  if (!res) {
    std::cout << "Error" << std::endl;
  }

  while (true) {
    sleep(10);
  }

  return 0;
}
