#include "pink/server.h"

#include <unistd.h>
#include <iostream>

#include "pink/eventbase_loop.h"

namespace pink {

bool Server::Start() {
  // sanitize opts

  int ret = opts_.worker_threads->Start();
  if (ret != 0) {
    return false;
  }
  ret = opts_.accept_thread->Start();
  if (ret != 0) {
    return false;
  }

  if (!dispatcher_.Bind()) {
    return false;
  }

  return true;
}

}  // namespace pink
