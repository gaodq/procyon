#pragma once

#include <string>
#include <memory>

#include "procyon/io_thread.h"

namespace procyon {

class Connection;
class ConnectionFactory;

struct ServerOptions {
  std::string listen_ip = "0.0.0.0";

  int port = 8089;

  std::shared_ptr<ConnectionFactory> conn_factory;

  std::shared_ptr<IOThread> accept_thread =
    std::shared_ptr<IOThread>(new IOThread);

  std::shared_ptr<IOThreadPool> worker_threads =
    std::shared_ptr<IOThreadPool>(new IOThreadPool(1));

  std::function<void(const Connection*)> error_callback;

  std::function<void(const Connection*)> close_callback;

  int connection_idle_timeout_s = 180; // 3 minutes
};

struct ClientOptions {

  int send_timeout_ms;
  int recv_timeout_ms;
  int connect_timeout_ms;
};

}  // namespace procyon

