#include "procyon/server.h"
#include "procyon/options.h"
#include "procyon/http_proto.h"
#include "procyon/io_thread.h"
#include "procyon/bg_thread.h"

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

procyon::BGThread worker_threads(4);

class CustomHandler : public procyon::HTTPMsgHandler {
 public:
  virtual void OnNewRequest(procyon::ConnectionPtr conn,
                                const procyon::HTTPRequest& req) override {
    if (req.headers.count("expect")) {
      WriteHeaders(conn, HTTP_STATUS_CONTINUE, headers_, 0);
      return;
    }
    std::cout << "* Method: " << http_method_str(req.method) << "\n";
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

  }

  virtual void OnBody(procyon::ConnectionPtr conn,
                      const char* data, size_t length) override {
    if (status_ == -1) {
      return;
    }
    std::cout << "length: " << length << std::endl;
    std::string content("error");
    WriteHeaders(conn, HTTP_STATUS_BAD_REQUEST, headers_, content.size());
    WriteContent(conn, content);
    status_ = -1;
  }

  virtual void OnComplete(procyon::ConnectionPtr conn) override {
    std::cout << "OnComplete" << std::endl;
    std::string content("nihaoa");
    WriteHeaders(conn, HTTP_STATUS_OK, headers_, content.size());
    WriteContent(conn, content);
  }

 private:
    std::unordered_map<std::string, std::string> headers_;
    int status_;
};

class MyConnFactory : public procyon::ConnectionFactory {
 public:
  std::shared_ptr<procyon::Connection> NewConnection() override {
    return std::make_shared<procyon::HTTPConn>(new CustomHandler);
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
    sleep(10);
  }

  return 0;
}
