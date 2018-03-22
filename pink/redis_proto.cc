#include "pink/redis_proto.h"

#include <cstdlib>

namespace pink {

bool RedisMsgHandler::HandleNewLine(Connection* conn, const std::string& line) {
  bool res = true;
  if (multibulk_len_ == -1) {
    multibulk_len_ = std::atoi(line.data() + 1);
    return res;
  } else if (bulk_len_ == -1) {
    bulk_len_ = std::atoi(line.data() + 1);
    return res;
  } else {
    if (command_.empty()) {
      command_.assign(line);
    } else {
      args_.push_back(line);
    }
    bulk_len_ = -1;
    multibulk_len_--;
  }
  if (multibulk_len_ == 0) {

    res = HandleRedisMsg(conn, command_, args_);
    command_.clear();
    args_.clear();
    multibulk_len_ = -1;
  }
  return res;;
}

}  // namespace pink
