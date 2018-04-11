#include "procyon/redis_proto.h"

#include <cstdlib>

namespace procyon {

void RedisMsgHandler::ProcessInlineBuffer(Connection* conn,
                                          std::unique_ptr<IOBuf>&& line) {
}

void RedisMsgHandler::ProcessMultibulkBuffer(Connection* conn,
                                             std::unique_ptr<IOBuf>&& line) {
  if (multibulk_len_ == -1) {
    std::string num = line->ToString();
    multibulk_len_ = std::atoi(num.data() + 1);
    return;
  } else if (bulk_len_ == -1) {
    std::string num = line->ToString();
    bulk_len_ = std::atoi(num.data() + 1);
    return;
  } else {
    args_.push_back(std::move(line));
    bulk_len_ = -1;
    multibulk_len_--;
  }
  if (multibulk_len_ == 0) {
    HandleRedisMsg(conn, args_);
    args_.clear();
    multibulk_len_ = -1;
  }
}

void RedisMsgHandler::HandleNewLine(Connection* conn,
                                    std::unique_ptr<IOBuf>&& line) {
  if (line->ByteAt(0) == '*' || multibulk_len_ != -1) {
    ProcessMultibulkBuffer(conn, std::move(line));
  } else {
    ProcessInlineBuffer(conn, std::move(line));
  }
}

}  // namespace procyon
