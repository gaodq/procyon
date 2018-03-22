#include "pink/linebased_proto.h"

namespace pink {

bool LineBasedConn::OnDataAvailable(size_t size) {
  size_t i = 0;
  size_t last_line = 0;
  while (true) {
    for (; i < size; i++) {
      if (buffer_[i] == '\r') {
        break;
      }
    }
    if (i == size) {
      break;
    }

    std::string new_line(buffer_.data() + last_line, i - last_line);
    if (!handler_->HandleNewLine(this, new_line)) {
      return false;
    }

    i += 2;
    last_line = i;
  }

  if (buffer_.size() > last_line + 1) {
    std::string remain(buffer_.data() + last_line + 2,
                       buffer_.size() - last_line - 2);
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
