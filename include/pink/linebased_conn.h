#ifndef PINK_LINEBASED_PROTO_H_
#define PINK_LINEBASED_PROTO_H_

#include <memory>

#include "pink/connection.h"

namespace pink {

class LineBasedConn : public Connection {
 public:
  struct LineMsgHandler {
    virtual ~LineMsgHandler() {}

    virtual void HandleMessage(Connection* conn, const std::string& line) {}

    void Write(Connection* conn, const std::string& msg) {
      conn->Write(msg.data(), msg.size());
    }
  };

  explicit LineBasedConn(LineMsgHandler* handler)
      : handler_(handler) {
  }

  bool OnDataAvailable(size_t size) override;

 private:
  LineMsgHandler* handler_;
};

}  // namespace pink
#endif  // PINK_LINEBASED_PROTO_H_
