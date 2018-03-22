#pragma once

#include <memory>

#include "pink/connection.h"
#include "pink/linebased_proto.h"

namespace pink {

class RedisMsgHandler : public LineMsgHandler {
 public:
  RedisMsgHandler()
      : multibulk_len_(-1),
        bulk_len_(-1) {
  }
  virtual ~RedisMsgHandler() {}

  bool HandleNewLine(Connection* conn, const std::string& line) override;

  virtual bool HandleRedisMsg(Connection* conn, const std::string& command,
                              const std::vector<std::string>& args) {
    return true;
  }

  void Write(Connection* conn, const std::string& msg) {
    conn->Write(msg.data(), msg.size());
  }

 private:
  std::string command_;
  std::vector<std::string> args_;

  int multibulk_len_;
  int bulk_len_;
};

}  // namespace pink
