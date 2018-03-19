#pragma once

#include <memory>

#include "pink/connection.h"

namespace pink {

const size_t kDefaultBufferSize = 16 * 1024;  // 16KB

class LineMsgHandler {
 public:
  virtual ~LineMsgHandler() {}

  virtual void HandleMessage(Connection* conn, const std::string& line) {}

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

  bool OnDataAvailable(size_t size) override;

  void GetReadBuffer(void** buffer, size_t* len) override;

 private:
  LineMsgHandler* handler_;

  std::string buffer_;
};

}  // namespace pink
