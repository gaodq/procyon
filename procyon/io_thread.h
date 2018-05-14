#pragma once

#include <memory>
#include <vector>

#include "procyon/thread.h"
#include "procyon/eventbase_loop.h"

namespace procyon {

class IOThread : public Thread {
 public:
  IOThread();
  IOThread(std::shared_ptr<EventbaseLoop> evp);

  std::shared_ptr<EventbaseLoop> event_loop() { return event_loop_; }

  int Start();
  int Stop();

  // Must be set before start
  void SetThreadName(const std::string& name);

 private:
  std::shared_ptr<EventbaseLoop> event_loop_;
};

class IOThreadPool {
 public:
  explicit IOThreadPool(int thread_num = 0);

  int Start();
  int Stop();

  class Observer {
   public:
    virtual void ThreadStarted(pthread_t pid) = 0;
    virtual void ThreadStoped(pthread_t pid) = 0;

    virtual ~Observer() = default;
  };

  void AddObserver(std::shared_ptr<Observer> o);
  void RemoveObserver(std::shared_ptr<Observer> o);

  std::shared_ptr<IOThread> NextThread();

  // Must be set before start
  void SetThreadName(const std::string& name);

 private:
  std::vector<std::shared_ptr<Observer>> observers_;
  std::vector<std::shared_ptr<IOThread>> threads_;
  int thread_num_;
  int index_;
};

}  // namespace procyon
