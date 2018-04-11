#include "procyon/server.h"
#include "procyon/options.h"
#include "procyon/connection.h"
#include "procyon/redis_proto.h"
#include "procyon/io_thread.h"
#include "procyon/bg_thread.h"

#include <unistd.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

procyon::BGThread worker_threads(2);

volatile uint64_t g_req_num = 0;

class CustomHandler : public procyon::RedisMsgHandler {
 public:
  struct RedisArgs {
    procyon::Connection* conn;
  };

  static void HandleDBRequest(void* arg) {
    usleep(5000);
    RedisArgs* redis_args = reinterpret_cast<RedisArgs*>(arg);
    redis_args->conn->Write("+OK\r\n", 5);
    delete redis_args;
  }

  virtual void HandleRedisMsg(
      procyon::Connection* conn,
      const std::vector<std::string>& args) override {
    g_req_num++;
    if (args.size() > 1) {
      const std::string& key = args[1];

      if (key == "slowdown") {
        RedisArgs* redis_args = new RedisArgs;
        redis_args->conn = conn;
        worker_threads.Schedule(HandleDBRequest, redis_args);
        return;
      }
    }

    Write(conn, "+OK\r\n");
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
  opts.port = 6379;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.worker_threads = std::make_shared<procyon::IOThreadPool>(4);

  worker_threads.Start();

  procyon::Server server(opts);
  bool res = server.Start();
  if (!res) {
    std::cout << "Error" << std::endl;
  }

  while (true) {
    auto start = std::chrono::system_clock::now();
    int prv = g_req_num;
    sleep(1);
    printf("request count: %d\n", g_req_num);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> diff = end - start;
    printf("Average qps: %lf\n", (double)(g_req_num - prv) / (diff.count() / 1000));
  }

  return 0;
}
