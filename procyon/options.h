#pragma once

#include <string>
#include <memory>

#include "procyon/io_thread.h"
#include "procyon/connection.h"

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

  std::function<void(ConnectionPtr)> error_callback;

  std::function<void(ConnectionPtr)> newconn_callback;

  std::function<void(ConnectionPtr)> close_callback;

  int connection_idle_timeout_s = 180; // 3 minutes
};

struct ClientOptions {
  std::string remote_ip = "0.0.0.0";
  int remote_port = 8089;

  std::string local_ip;

  std::shared_ptr<ConnectionFactory> conn_factory;

  std::shared_ptr<IOThread> io_thread =
    std::shared_ptr<IOThread>(new IOThread);

  std::function<void(ConnectionPtr)> error_callback;

  std::function<void(ConnectionPtr)> close_callback;

  int connect_timeout_ms = 1000;
};

}  // namespace procyon

