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

volatile uint64_t g_req_num = 0;

class CustomHandler : public procyon::LineMsgHandler {
 public:
  void HandleNewLine(procyon::Connection* conn,
                     std::unique_ptr<procyon::IOBuf>&& line) override {
    // std::cout << "receive: " << line->ToString() << std::endl;
    g_req_num++;
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
  opts.worker_threads = std::make_shared<procyon::IOThreadPool>(24);

  procyon::Server server(opts);
  bool res = server.Start();
  if (!res) {
    return -1;
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
