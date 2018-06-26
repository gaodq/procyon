#include <unistd.h>
#include <iostream>

#include "procyon/client.h"
#include "procyon/options.h"
#include "procyon/fixlength_proto.h"

#include "example.pb.h"

class ProtobufMsgHandler : public procyon::FixLengthMsgHandler {
 public:
  virtual void HandleNewMsg(
      procyon::ConnectionPtr conn,
      std::unique_ptr<procyon::IOBuf>&& msg) override {
    example::Ping ping;
    bool res = ping.ParseFromString(msg->ToString());
    if (!res) {
      std::cout << "Parse failed" << std::endl;
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
  procyon::ClientOptions opts;
  opts.remote_ip = "127.0.0.1";
  opts.remote_port = 8099;
  opts.conn_factory = std::make_shared<MyConnFactory>();

  opts.io_thread->Start();
  procyon::Client client(opts);
  bool res = client.Connect();
  if (!res) {
    return -1;
  }

  example::Ping ping;
  ping.set_address("hello");
  ping.set_port(9011);
  std::string msg;
  ping.SerializeToString(&msg);
  procyon::FixLengthMsgHandler::WriteMsg(client.conn(), msg).get();

  return 0;
}
