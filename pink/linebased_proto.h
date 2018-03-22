#pragma once

#include <memory>

#include "pink/connection.h"

namespace pink {

const size_t kDefaultBufferSize = 16 * 1024;  // 16KB

class LineMsgHandler {
 public:
  virtual ~LineMsgHandler() {}

  virtual bool HandleNewLine(Connection* conn, const std::string& line) {
    return true;
  }

  void Write(Connection* conn, const std::string& msg) {
    conn->Write(msg.data(), msg.size());
  }
};

class LineBasedConn : public Connection {
 public:
  explicit LineBasedConn(LineMsgHandler* handler)
      : handler_(handler) {
    buffer_.reserve(kDefaultBufferSize);
  }

  virtual bool OnDataAvailable(size_t size) override;

  virtual void GetReadBuffer(void** buffer, size_t* len) override;

 private:
  LineMsgHandler* handler_;

  std::string buffer_;
};

}  // namespace pink
