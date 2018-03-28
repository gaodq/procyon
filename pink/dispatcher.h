#pragma once

#include <unordered_map>
#include <mutex>

#include "pink/options.h"
#include "pink/eventbase_loop.h"
#include "pink/io_thread.h"
#include "pink/server_socket.h"

namespace pink {

class Connection;

class Dispatcher {
 public:
  explicit Dispatcher(const ServerOptions& options);

  bool Bind();

  void OnNewConnection();
  void OnConnClosed(const Connection* conn);
  void OnConnError(const Connection* conn);

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
  std::unordered_map<int, std::shared_ptr<Connection>> connections_;
  std::mutex conn_mu_;

  AcceptHandler ac_handler_;
};

}  // namespace pink
