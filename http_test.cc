#include "pink/server.h"
#include "pink/options.h"
#include "pink/http_proto.h"
#include "pink/io_thread.h"
#include "pink/bg_thread.h"

#include <unistd.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

pink::BGThread worker_threads(4);

class CustomHandler : public pink::HTTPMsgHandler {
 public:
  virtual void HandleNewRequest(pink::Connection* conn,
                                const pink::HTTPRequest& req) override {
    std::cout << "* Method: " <<
      http_method_str(static_cast<enum http_method>(req.method)) << "\n";
    std::cout << "* URL: " << req.req_url << "\n";
    std::cout << "* Path: " << req.path << "\n";
    std::cout << "* Content-Type: " << req.content_type << "\n";

    std::cout << "* Query parameters: " << "\n";
    for (auto& item : req.query_params) {
      std::cout << "   " << item.first << ": " << item.second << "\n";
    }

    std::cout << "* Headers: " << "\n";
    for (auto& item : req.headers) {
      std::cout << "   " << item.first << ": " << item.second << "\n";
    }

    std::cout << "* Post values: " << "\n";
    for (auto& item : req.postform_values) {
      std::cout << "   " << item.first << ": " << item.second << "\n";
    }
    Write(conn, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
  }

  virtual void OnBody(pink::Connection* conn,
                      const char* data, size_t length) override {
    std::cout << std::string(data, length) << std::endl;
  }
};

class MyConnFactory : public pink::ConnectionFactory {
 public:
  std::shared_ptr<pink::Connection> NewConnection() override {
    return std::make_shared<pink::HTTPConn>(new CustomHandler);
  }
};

int main() {
  pink::ServerOptions opts;
  opts.port = 6379;
  opts.conn_factory = std::make_shared<MyConnFactory>();
  opts.worker_threads = std::make_shared<pink::IOThreadPool>(4);

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
