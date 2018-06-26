#pragma once

#include <memory>

#include "procyon/iobuf.h"
#include "procyon/connection.h"

namespace procyon {

class FixLengthMsgHandler {
 public:
  virtual ~FixLengthMsgHandler() = default;

  virtual void HandleNewMsg(
      ConnectionPtr conn, std::unique_ptr<IOBuf>&& msg) {
  }

  static std::future<bool> WriteMsg(
      ConnectionPtr conn, const std::string& msg) {
    std::string content;
    int size = msg.size();
    content.append(reinterpret_cast<char*>(&size), 4);
    content.append(msg);
    return conn->Write(content.data(), content.size());
  }
};

/*
 *  | 4 - bytes | [length] - bytes |
 *  |  length   |     contents     |
 */

class FixLengthConn : public Connection {
 public:
  explicit FixLengthConn(FixLengthMsgHandler* handler)
      : handler_(handler),
        msg_length_(0) {
  }

  virtual void OnDataAvailable(size_t size) override;

  virtual void GetReadBuffer(void** buffer, size_t* size) override;

 private:
  std::unique_ptr<FixLengthMsgHandler> handler_;
  IOBuf buffer_;
  int msg_length_;
};

}  // namespace procyon
