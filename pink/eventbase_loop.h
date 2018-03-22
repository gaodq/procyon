#pragma once

#include <sys/epoll.h>

#include "pink/thread.h"

namespace pink {

class EventbaseLoop;

class EventHandler {
 public:
  enum EventFlags {
    kRead = EPOLLIN,
    kWrite = EPOLLOUT,
    kReadWrite = EPOLLIN | EPOLLOUT,
    kError = EPOLLERR | EPOLLHUP,
  };

  EventHandler() = default;
  virtual ~EventHandler() {
    UnRegisterHandler();
  }

  bool RegisterHandler(std::shared_ptr<EventbaseLoop> l, int fd);

  virtual void HandleReady(uint32_t events) = 0;

  bool EnableRead();
  bool EnableWrite();
  bool DisableWrite();

 private:
  bool UnRegisterHandler();

  std::shared_ptr<EventbaseLoop> event_loop_;
  int fd_;
  struct epoll_event event_;
};

class EventbaseLoop : public Thread::Runnable {
 public:
  EventbaseLoop();
  ~EventbaseLoop();

  // Runnable
  void run() override;

  void RunInLoop(std::function<void()> fn);

  void Stop() { should_stop_ = true; }

  int epfd() { return epfd_; }

 private:
  int epfd_;
  volatile bool should_stop_;
};

}  // namespace pink
