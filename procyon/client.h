#pragma once

#include "procyon/connection.h"
#include "procyon/options.h"

namespace procyon {

class Client : public ConnectionManager {
 public:
  explicit Client(const ClientOptions& options);

  bool Connect();

  void OnConnClosed(int) override;

  void OnConnError(int) override;

  ConnectionPtr conn() { return conn_; }

 private:
  ClientOptions options_;
  ConnectionPtr conn_;
};

}  // namespace procyon
