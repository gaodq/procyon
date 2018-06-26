#pragma once

#include <unordered_map>
#include <mutex>

#include "procyon/options.h"
#include "procyon/connection.h"
#include "procyon/eventbase_loop.h"
#include "procyon/io_thread.h"
#include "procyon/server_socket.h"

namespace procyon {

class Dispatcher : public ConnectionManager {
 public:
  explicit Dispatcher(const ServerOptions& options);

  bool Bind();

  void OnNewConnection();
  void OnConnClosed(int conn_fd) override;
  void OnConnError(int conn_fd) override;

  struct AcceptHandler : EventHandler {
    explicit AcceptHandler(Dispatcher* d) : dispacher(d) {}

    void HandleReady(uint32_t /*events*/) override {
      dispacher->OnNewConnection();
    }

    Dispatcher* dispacher;
  };

 private:
  const ServerOptions opts_;

  ServerSocket server_socket_;
  std::unordered_map<int, ConnectionPtr> connections_;
  std::mutex conn_mu_;

  AcceptHandler ac_handler_;
};

}  // namespace procyon
