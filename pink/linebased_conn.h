#ifndef PINK_LINEBASED_CONN_H_
#define PINK_LINEBASED_CONN_H_

#include <memory>

#include "pink/connection.h"

namespace pink {

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
  }

  bool OnDataAvailable(size_t size) override;

 private:
  LineMsgHandler* handler_;
};

}  // namespace pink
#endif  // PINK_LINEBASED_PROTO_H_
