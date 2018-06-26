#include "procyon/fixlength_proto.h"

#include "procyon/xdebug.h"

namespace procyon {

void FixLengthConn::OnDataAvailable(size_t size) {
  log_info("receive data size: %lu", size);
  buffer_.PostAllocate(size);
  while (buffer_.length() > 0) {
    if (msg_length_ == 0 && buffer_.length() >= 4) {
      char buf[4];
      buf[0] = buffer_.ByteAt(0);
      buf[1] = buffer_.ByteAt(1);
      buf[2] = buffer_.ByteAt(2);
      buf[3] = buffer_.ByteAt(3);
      msg_length_ = *reinterpret_cast<int*>(buf);
      buffer_.TrimStart(4);
    }

    if (buffer_.length() >= static_cast<size_t>(msg_length_)) {
      std::unique_ptr<IOBuf> new_msg = buffer_.Split(msg_length_);
      handler_->HandleNewMsg(shared_from_this(), std::move(new_msg));
      buffer_.TrimStart(msg_length_);
      msg_length_ = 0;
    } else {
      break;
    }
  }
}

void FixLengthConn::GetReadBuffer(void** buffer, size_t* len) {
  auto mem = buffer_.PreAllocate();
  *buffer = mem.first;
  *len = mem.second;
}

}  // namespace procyon
