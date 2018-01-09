#ifndef PINK_SERVER_H_
#define PINK_SERVER_H_

#include <string>

#include "pink/connection.h"

namespace pink {

class IOThread;
class IOThreadPool;

struct ServerOptions {
  std::string listen_ip;
  int port;
  std::shared_ptr<ConnectionFactory> conn_factory;
  std::shared_ptr<IOThread> accept_thread;
  std::shared_ptr<IOThreadPool> worker_threads;
};

class Server {
 public:
  int Bind(const std::string& ip, int port);
  int Bind(int port) {
    return Bind("0.0.0.0", port);
  }
  int StartUntilInterrupt();
  int Interrupt();
};

}  // namespace pink

#endif  // PINK_SERVER_H_
