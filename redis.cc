#include "pink/server.h"
#include "pink/options.h"
#include "pink/connection.h"
#include "pink/redis_proto.h"
#include "pink/io_thread.h"

#include <unistd.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

class CustomHandler : public pink::RedisMsgHandler {
 public:
  virtual bool HandleRedisMsg(pink::Connection* conn,
                              const std::string& command,
                              const std::vector<std::string>& args) override {

    usleep(500);
    Write(conn, "+OK\r\n");

    return true;
  }
};

class MyConnFactory : public pink::ConnectionFactory {
 public:
  std::shared_ptr<pink::Connection> NewConnection() override {
    return std::make_shared<pink::LineBasedConn>(new CustomHandler);
  }
};

int main() {
  pink::ServerOptions opts;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  pink::Server server(opts);
  bool res = server.Start();
  if (!res) {
    std::cout << "Error" << std::endl;
  }

  while (true) {
    sleep(10);
  }

  return 0;
}
