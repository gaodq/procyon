#pragma once

#include <string>
#include <memory>

#include "pink/dispatcher.h"
#include "pink/io_thread.h"
#include "pink/options.h"

namespace pink {

class Server {
 public:
  Server(const ServerOptions& opts)
      : opts_(opts),
        dispatcher_(opts) {
  }

  bool Start();

 private:
  const ServerOptions opts_;
  Dispatcher dispatcher_;
};

}  // namespace pink
