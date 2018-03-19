#pragma once

#include <memory>
#include <vector>

#include "pink/thread.h"
#include "pink/eventbase_loop.h"

namespace pink {

class IOThread : public Thread {
 public:
  IOThread();
  IOThread(std::shared_ptr<EventbaseLoop> evp);

  std::shared_ptr<EventbaseLoop> event_loop() { return event_loop_; }

  int Start();
  int Stop();

 private:
  std::shared_ptr<EventbaseLoop> event_loop_;
};

class IOThreadPool {
 public:
  explicit IOThreadPool(int thread_num = 0);

  int Start();
  int Stop();

  std::shared_ptr<IOThread> NextThread();

 private:
  std::vector<std::shared_ptr<IOThread>> threads_;
  int thread_num_;
  int index_;
};

}  // namespace pink
