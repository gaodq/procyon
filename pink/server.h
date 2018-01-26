#ifndef PINK_SERVER_H_
#define PINK_SERVER_H_

#include <string>

#include "pink/connection.h"

namespace pink {

class IOThread;
class IOThreadPool;
class Dispatcher;

struct ServerOptions {
  std::string listen_ip;
  int port;
  std::shared_ptr<ConnectionFactory> conn_factory;
  std::shared_ptr<IOThread> accept_thread;
  std::shared_ptr<IOThreadPool> worker_threads;
};

class Server {
 public:
  bool Start(const ServerOptions& opts);

 private:
  std::shared_ptr<Dispatcher> dispatcher_;
};

}  // namespace pink

#endif  // PINK_SERVER_H_
