#include "procyon/linebased_proto.h"
#include "procyon/xdebug.h"

namespace procyon {

void LineBasedConn::OnDataAvailable(size_t size) {
  log_info("receive data size: %lu", size);
  buffer_.PostAllocate(size);
  size_t pos = 0;
  while (pos < buffer_.length()) {
    if (buffer_.ByteAt(pos) == '\r') {
      std::unique_ptr<IOBuf> new_line = buffer_.Split(pos);
      handler_->HandleNewLine(shared_from_this(), std::move(new_line));
      buffer_.TrimStart(2);  // Trim \r\n
      pos = 0;
    } else {
      pos++;
    }
  }
}

void LineBasedConn::GetReadBuffer(void** buffer, size_t* len) {
  auto mem = buffer_.PreAllocate();
  *buffer = mem.first;
  *len = mem.second;
}

}  // namespace procyon
