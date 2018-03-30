#include "pink/server.h"
#include "pink/connection.h"
#include "pink/linebased_conn.h"
#include "pink/io_thread.h"

#include <unistd.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<int> rpc_num(0);

class CustomHandler : public pink::LineMsgHandler {
 public:
  void HandleMessage(pink::Connection* conn, std::unique_ptr<pink::IOBuf>&& msg) override {
    // std::cout << "receive: " << msg << std::endl;
    rpc_num.fetch_add(1, std::memory_order_relaxed);
    Write(conn, msg + "\r\n");
  }
};

class MyConnFactory : public pink::ConnectionFactory {
 public:
  std::shared_ptr<pink::Connection> NewConnection() override {
    return std::make_shared<pink::LineBasedConn>(new CustomHandler);
  }
};

void func() {
  while (true) {
    auto start = std::chrono::steady_clock::now();
    int last_num = rpc_num.load(std::memory_order_relaxed);
    sleep(1);
    auto end = std::chrono::steady_clock::now();
    int num = rpc_num.load(std::memory_order_relaxed);
    std::chrono::duration<double, std::milli> diff = end - start;
    printf("Qps: %lf\n", (double)(num - last_num) / diff.count() * 1000);
  }
}

int main() {
  pink::Server server;
  pink::ServerOptions opts;
  opts.listen_ip = "0.0.0.0";
  opts.port = 8089;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.accept_thread = std::make_shared<pink::IOThread>();
  opts.worker_threads = std::make_shared<pink::IOThreadPool>(4);
  bool res = server.Start(opts);
  if (!res) {
    std::cout << "Error" << std::endl;
  }

  std::thread t1(func);

  t1.join();

  return 0;
}
