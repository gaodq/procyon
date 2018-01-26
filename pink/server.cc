#include "pink/server.h"

#include <unistd.h>
#include <iostream>

#include "pink/dispatcher.h"
#include "pink/io_thread.h"
#include "pink/eventbase_loop.h"

namespace pink {

bool Server::Start(const ServerOptions& opts) {
  // sanitize opts

  dispatcher_ = std::make_shared<Dispatcher>(opts);

  int ret = opts.worker_threads->Start();
  if (ret != 0) {
    return false;
  }
  ret = opts.accept_thread->Start();
  if (ret != 0) {
    return false;
  }

  if (!dispatcher_->Bind()) {
    return false;
  }

  return true;
}

}  // namespace pink
