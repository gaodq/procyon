#include "procyon/eventbase_loop.h"

#include <linux/version.h>
#include <fcntl.h>

#include "procyon/xdebug.h"
#include "procyon/util.h"

namespace procyon {

bool EventHandler::RegisterHandler(std::shared_ptr<EventbaseLoop> l, int fd) {
  // Ensure unregister
  event_loop_ = l;
  fd_ = fd;

  return EnableRead();
}

void EventHandler::UnRegisterHandler() {
  int ret = epoll_ctl(event_loop_->epfd(), EPOLL_CTL_DEL, fd_, nullptr);
  if (ret < 0) {
    log_warn("Failed delete fd: %d event from epfd_:%d",
             fd_, event_loop_->epfd());
  }
}

bool EventHandler::EnableRead() {
  event_.data.ptr = reinterpret_cast<void*>(this);
  event_.events = EPOLLIN;

  int ret = epoll_ctl(event_loop_->epfd(), EPOLL_CTL_ADD, fd_, &event_);
  if (ret < 0) {
    log_warn("Failed add fd: %d event to epfd_:%d", fd_, event_loop_->epfd());
    return false;
  }
  return true;
}

bool EventHandler::EnableWrite() {
  event_.data.ptr = reinterpret_cast<void*>(this);
  event_.events = EPOLLOUT | EPOLLIN;
  int ret = epoll_ctl(event_loop_->epfd(), EPOLL_CTL_MOD, fd_, &event_);
  if (ret < 0) {
    log_warn("Failed add fd: %d event to epfd_:%d", fd_, event_loop_->epfd());
    return false;
  }
  return true;
}

bool EventHandler::DisableWrite() {
  event_.data.ptr = reinterpret_cast<void*>(this);
  event_.events = EPOLLIN;
  int ret = epoll_ctl(event_loop_->epfd(), EPOLL_CTL_MOD, fd_, &event_);
  if (ret < 0) {
    log_warn("Failed add fd: %d event to epfd_:%d", fd_, event_loop_->epfd());
    return false;
  }
  return true;
}

EventbaseLoop::EventbaseLoop()
      : epfd_(-1),
        should_stop_(false) {
  epfd_ = epoll_create1(EPOLL_CLOEXEC);

  if (epfd_ < 0) {
    log_err("epoll create fail");
    exit(1);
  }

  if (pipe(exit_fd_) < 0) {
    log_err("pipe create fail");
    exit(1);
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = exit_fd_[0];

  int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, exit_fd_[0], &event);
  if (ret < 0) {
    log_warn("Failed add fd: %d event to epfd_:%d", exit_fd_[0], epfd_);
    exit(1);
  }
}

EventbaseLoop::~EventbaseLoop() {
  close(epfd_);
  close(exit_fd_[0]);
  close(exit_fd_[1]);
}

const int kPinkMaxClients = 10240;
const int kPinkEpollTimeout = 1000;

void EventbaseLoop::Stop() {
  should_stop_ = true;
  write(exit_fd_[1], "", 1);
}

void EventbaseLoop::run() {
  struct epoll_event e[kPinkMaxClients];
  while (!should_stop_) {
    {
    std::vector<std::pair<std::function<void()>, bool>> cbs, next_cbs;
    std::unique_lock<std::mutex> l(callbacks_mu_);
    callbacks_.swap(cbs);
    l.unlock();
    for (auto item : cbs) {
      auto fn = item.first;
      bool once = item.second;
      if (!once) {
        next_cbs.push_back(item);
      }
      fn();
    }
    l.lock();
    callbacks_.insert(callbacks_.end(), next_cbs.begin(), next_cbs.end());
    }

    int n = epoll_wait(epfd_, e, kPinkMaxClients, kPinkEpollTimeout);
    if (should_stop_) {
      break;
    }

    for (int i = 0; i < n; i++) {
      if (e[i].data.fd == exit_fd_[0]) {
        should_stop_ = true;
        break;
      }
      EventHandler* handler = reinterpret_cast<EventHandler*>(e[i].data.ptr);
      handler->HandleReady(e[i].events);
    }
    // n little than zero, continue
  }
}

void EventbaseLoop::RunInLoop(std::function<void()> fn, bool once) {
  std::lock_guard<std::mutex> l(callbacks_mu_);
  callbacks_.push_back(std::make_pair(fn, once));
}

}  // namespace procyon
