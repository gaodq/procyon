#ifndef PINK_DISPATCHER_H_
#define PINK_DISPATCHER_H_

#include <unordered_map>

#include "pink/connection.h"
#include "pink/server.h"
#include "pink/eventbase_loop.h"
#include "pink/io_thread.h"
#include "pink/server_socket.h"

namespace pink {

class Dispatcher {
 public:
  explicit Dispatcher(const ServerOptions& options);

  bool Bind();

  void OnNewConnection();
  void OnConnClosed(Connection* conn);

  struct AcceptHandler : EventHandler {
    explicit AcceptHandler(Dispatcher* d) : dispacher(d) {}

    void HandleReady(uint32_t /*events*/) override {
      dispacher->OnNewConnection();
    }

    Dispatcher* dispacher;
  };

  std::function<void(Connection*)> error_cb_;
  std::function<void(Connection*)> close_cb_;

 private:
  const std::string ip_;
  const int port_;

  std::shared_ptr<IOThread> accept_thread_;
  std::shared_ptr<IOThreadPool> worker_threads_;

  ServerSocket server_socket_;
  std::unordered_map<int, std::shared_ptr<Connection>> connections_;

  AcceptHandler ac_handler_;
  std::shared_ptr<ConnectionFactory> conn_factory_;
};

}  // namespace pink
#endif  // PINK_CONN_MANAGER_H_
