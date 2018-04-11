#pragma once

#include <memory>

#include "procyon/connection.h"
#include "procyon/linebased_proto.h"

namespace procyon {

class RedisMsgHandler : public LineMsgHandler {
 public:
  RedisMsgHandler()
      : multibulk_len_(-1),
        bulk_len_(-1) {
  }
  virtual ~RedisMsgHandler() {}

  void HandleNewLine(Connection* conn, std::unique_ptr<IOBuf>&& line) override;

  void ProcessInlineBuffer(Connection* conn, std::unique_ptr<IOBuf>&& line);
  void ProcessMultibulkBuffer(Connection* conn, std::unique_ptr<IOBuf>&& line);

  virtual void HandleRedisMsg(
      Connection* conn,
      const std::vector<std::unique_ptr<IOBuf>>& args) {
  }

  void Write(Connection* conn, const std::string& msg) {
    conn->Write(msg.data(), msg.size());
  }

 private:
  std::vector<std::unique_ptr<IOBuf>> args_;

  int multibulk_len_;
  int bulk_len_;
};

}  // namespace procyon
