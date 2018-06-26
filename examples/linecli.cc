#include <unistd.h>
#include <iostream>

#include "procyon/client.h"
#include "procyon/options.h"
#include "procyon/linebased_proto.h"

class CustomHandler : public procyon::LineMsgHandler {
 public:
  void HandleNewLine(procyon::ConnectionPtr conn,
                     std::unique_ptr<procyon::IOBuf>&& line) override {
    std::cout << "Client receive: " << line->ToString() << std::endl;
  }
};

class MyConnFactory : public procyon::ConnectionFactory {
 public:
  std::shared_ptr<procyon::Connection> NewConnection() override {
    return std::make_shared<procyon::LineBasedConn>(new CustomHandler);
  }
};

int main() {
  procyon::ClientOptions opts;
  opts.remote_ip = "127.0.0.1";
  opts.remote_port = 8088;
  opts.conn_factory = std::make_shared<MyConnFactory>();

  opts.io_thread->Start();
  procyon::Client client(opts);
  bool res = client.Connect();
  if (!res) {
    return -1;
  }

  std::string msg("hello\r\n");
  while (true) {
    procyon::LineMsgHandler::WriteLine(client.conn(), msg);
  }

  return 0;
}
