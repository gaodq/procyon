#include "procyon/bg_thread.h"

#include <sys/time.h>
#include <chrono>

#include "procyon/xdebug.h"

namespace procyon {

BGThread::BGThread(size_t thread_num, size_t max_queue)
    : running_(false),
      thread_num_(thread_num),
      max_queue_(max_queue) {
  for (size_t i = 0; i < thread_num_; i++) {
    std::unique_ptr<Thread> t(new Thread(std::make_shared<Worker>(this)));
    thread_list_.push_back(std::move(t));
  }
}

int BGThread::Start() {
  int res = 0;
  running_ = true;
  for (size_t i = 0; i < thread_num_; i++) {
    res = thread_list_[i]->Start();
    if (res != 0) {
      break;
    }
  }
  return res;
}

void BGThread::Stop() {
  if (!running_) {
    return;
  }
  running_ = false;
  rsignal_.notify_one();
  wsignal_.notify_one();
  Join();
}

int BGThread::Join() {
  int res = 0;
  for (size_t i = 0; i < thread_num_; i++) {
    res = thread_list_[i]->Join();
    if (res != 0) {
      break;
    }
  }
  return res;
}

void BGThread::Schedule(Func fn, void* arg) {
  std::unique_lock<std::mutex> lk(mu_);
  wsignal_.wait(lk, [this]{
    return (queue_.size() < max_queue_ || !running_);
  });
  if (running_) {
    queue_.push(BGItem(fn, arg));
    rsignal_.notify_one();
  }
}

void BGThread::DelaySchedule(uint64_t timeout_ms, Func fn, void* arg) {
  struct timeval now;
  gettimeofday(&now, nullptr);
  uint64_t exec_time;
  exec_time = now.tv_sec * 1000000 + timeout_ms * 1000 + now.tv_usec;

  std::unique_lock<std::mutex> lk(mu_);
  timer_queue_.push(TimerItem(exec_time, fn, arg));
  rsignal_.notify_one();
}

void BGThread::QueueSize(int* pri_size, int* qu_size) {
  std::lock_guard<std::mutex> l(mu_);
  *pri_size = timer_queue_.size();
  *qu_size = queue_.size();
}

void BGThread::QueueClear() {
  std::lock_guard<std::mutex> l(mu_);
  std::queue<BGItem>().swap(queue_);
  std::priority_queue<TimerItem>().swap(timer_queue_);
}

void BGThread::Worker::run() {
  while (bg_thread_->running_) {
    std::unique_lock<std::mutex> lk(bg_thread_->mu_);
    bg_thread_->rsignal_.wait(lk, [this]{ 
      return (!bg_thread_->queue_.empty() ||
              !bg_thread_->timer_queue_.empty() ||
              !bg_thread_->running_);
    });
    if (!bg_thread_->running_) {
      break;
    }
    if (!bg_thread_->timer_queue_.empty()) {
      struct timeval now;
      gettimeofday(&now, nullptr);

      TimerItem timer_item = bg_thread_->timer_queue_.top();
      uint64_t unow = now.tv_sec * 1000000 + now.tv_usec;
      if (unow / 1000 >= timer_item.exec_time / 1000) {
        Func fn = timer_item.func;
        void* arg = timer_item.arg;
        bg_thread_->timer_queue_.pop();
        lk.unlock();
        fn(arg);
        continue;
      } else if (bg_thread_->queue_.empty() && bg_thread_->running_) {
        std::chrono::microseconds t(timer_item.exec_time - unow);
        bg_thread_->rsignal_.wait_for(lk, t);
        continue;
      }
    }
    if (!bg_thread_->queue_.empty()) {
      Func fn = bg_thread_->queue_.front().func;
      void* arg = bg_thread_->queue_.front().arg;
      bg_thread_->queue_.pop();
      bg_thread_->wsignal_.notify_one();
      lk.unlock();
      fn(arg);
    }
  }
}

}  // namespace procyon
