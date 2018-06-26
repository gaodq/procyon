#include "procyon/server.h"
#include "procyon/options.h"
#include "procyon/fixlength_proto.h"
#include "procyon/io_thread.h"

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

#include "example.pb.h"

class ProtobufMsgHandler : public procyon::FixLengthMsgHandler {
 public:
  virtual void HandleNewMsg(
      procyon::ConnectionPtr conn,
      std::unique_ptr<procyon::IOBuf>&& msg) override {
    std::string m = msg->ToString();
    example::Ping ping;
    bool res = ping.ParseFromString(m);
    if (!res) {
      std::cout << "Parse failed" << std::endl;
      return;
    }
    std::cout << "Receive ping: " << ping.address() << std::endl;
  }
};

class MyConnFactory : public procyon::ConnectionFactory {
 public:
  std::shared_ptr<procyon::Connection> NewConnection() override {
    return std::make_shared<procyon::FixLengthConn>(new ProtobufMsgHandler);
  }
};

int main() {
  procyon::ServerOptions opts;
  opts.port = 8099;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.worker_threads = std::make_shared<procyon::IOThreadPool>(1);

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
