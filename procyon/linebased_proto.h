#pragma once

#include <memory>

#include "procyon/connection.h"
#include "procyon/iobuf.h"

namespace procyon {

class LineMsgHandler {
 public:
  virtual ~LineMsgHandler() = default;

  virtual void HandleNewLine(
      ConnectionPtr conn, std::unique_ptr<IOBuf>&& line) {
  }

  static std::future<bool> WriteLine(
      ConnectionPtr conn, const std::string& msg) {
    return conn->Write(msg.data(), msg.size());
  }
};

class LineBasedConn : public Connection {
 public:
  explicit LineBasedConn(LineMsgHandler* handler)
      : handler_(handler) {
  }

  virtual void OnDataAvailable(size_t size) override;

  virtual void GetReadBuffer(void** buffer, size_t* len) override;

 private:
  std::unique_ptr<LineMsgHandler> handler_;
  IOBuf buffer_;
};

}  // namespace procyon
