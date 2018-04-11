#pragma once

#include <string>
#include <memory>

#include "procyon/dispatcher.h"
#include "procyon/io_thread.h"
#include "procyon/options.h"

namespace procyon {

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

}  // namespace procyon
