#include "pink/server.h"
#include "pink/options.h"
#include "pink/connection.h"
#include "pink/redis_proto.h"
#include "pink/io_thread.h"
#include "pink/bg_thread.h"

#include <unistd.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

pink::BGThread worker_threads(4);

class CustomHandler : public pink::RedisMsgHandler {
 public:
  struct RedisArgs {
    pink::Connection* conn;
  };

  static void HandleDBRequest(void* arg) {
    usleep(5000);
    RedisArgs* redis_args = reinterpret_cast<RedisArgs*>(arg);
    redis_args->conn->Write("+OK\r\n", 5);
    delete redis_args;
  }

  virtual void HandleRedisMsg(
      pink::Connection* conn,
      const std::vector<std::unique_ptr<pink::IOBuf>>& args) override {
    if (args.size() > 1) {
      const std::string& key = args[1]->ToString();

      // if (key < "key:000000000010") {
      //   RedisArgs* redis_args = new RedisArgs;
      //   redis_args->conn = conn;
      //   worker_threads.Schedule(HandleDBRequest, redis_args);
      //   return;
      // }
    }

    Write(conn, "+OK\r\n");
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
  opts.port = 6379;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.worker_threads = std::make_shared<pink::IOThreadPool>(20);

  worker_threads.Start();

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
