#include "pink/linebased_conn.h"

namespace pink {

bool LineBasedConn::OnDataAvailable(size_t size) {
  size_t i = 0;
  for (; i < size; i++) {
    if (buffer_[i] == '\r') {
      break;
    }
  }

  std::string new_line(buffer_.data(), i);
  handler_->HandleMessage(this, new_line);

  if (buffer_.size() > i + 1) {
    std::string remain(buffer_.data() + i + 2, buffer_.size() - i - 2);
    remain.reserve(kDefaultBufferSize);
    buffer_.swap(remain);
  }
  
  return true;
}

void LineBasedConn::GetReadBuffer(void** buffer, size_t* len) {
  if (buffer_.capacity() == buffer_.size()) {
    buffer_.reserve(buffer_.size() * 2);
  }
  *len = buffer_.capacity() - buffer_.size();
  *buffer = const_cast<char*>(buffer_.data() + buffer_.size());
}

}  // namespace pink
