#pragma once

#include <sys/epoll.h>
#include <mutex>
#include <vector>

#include "procyon/thread.h"

namespace procyon {

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
  virtual ~EventHandler() = default;

  bool RegisterHandler(std::shared_ptr<EventbaseLoop> l, int fd);
  void UnRegisterHandler();

  virtual void HandleReady(uint32_t events) = 0;

  bool EnableRead();
  bool EnableWrite();
  bool DisableWrite();

 private:

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

  // TODO Timer
  void RunInLoop(std::function<void()> fn, bool once = true);

  void Stop();

  int epfd() { return epfd_; }

 private:
  int epfd_;
  volatile bool should_stop_;

  std::mutex callbacks_mu_;
  std::vector<std::pair<std::function<void()>, bool>> callbacks_;

  int exit_fd_[2];
};

}  // namespace procyon
