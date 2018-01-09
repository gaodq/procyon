#include "pink/linebased_conn.h"

namespace pink {

bool LineBasedConn::OnDataAvailable(size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (read_buf_[i] == '\r') {
      std::string new_line(read_buf_, i);
      handler_->HandleMessage(this, new_line);
    }
  }
  return true;
}

}  // namespace pink
