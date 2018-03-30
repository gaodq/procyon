#include "pink/linebased_proto.h"

namespace pink {

void LineBasedConn::OnDataAvailable(size_t size) {
  buffer_.PostAllocate(size);
  size_t pos = 0;
  while (pos < buffer_.length()) {
    if (buffer_.ByteAt(pos) == '\r') {
      std::unique_ptr<IOBuf> new_line = buffer_.Split(pos);
      handler_->HandleNewLine(this, std::move(new_line));
      buffer_.TrimStart(2);  // Trim \r\n
      pos = 0;
    } else {
      pos++;
    }
  }
}

void LineBasedConn::GetReadBuffer(void** buffer, size_t* len) {
  auto mem = buffer_.PreAllocate(kDefaultBufferSize);
  *buffer = mem.first;
  *len = mem.second;
}

}  // namespace pink
