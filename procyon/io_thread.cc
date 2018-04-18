#include <thread>

#include "procyon/io_thread.h"

namespace procyon {

IOThread::IOThread() : IOThread(std::make_shared<EventbaseLoop>()) {}

IOThread::IOThread(std::shared_ptr<EventbaseLoop> evp)
      : Thread(evp),
        event_loop_(evp) {
}

int IOThread::Start() {
  return Thread::Start();
}

int IOThread::Stop() {
  event_loop_->Stop();
  return Thread::Join();
}

IOThreadPool::IOThreadPool(int thread_num)
      : index_(0) {
  thread_num_ = thread_num == 0 ?
    std::thread::hardware_concurrency() : thread_num;
  for (int i = 0; i < thread_num; i++) {
    threads_.push_back(
      std::make_shared<IOThread>(std::make_shared<EventbaseLoop>()));
  }
}

int IOThreadPool::Start() {
  for (int i = 0; i < thread_num_; i++) {
    int ret = threads_[i]->Start();
    if (ret != 0) {
      return ret;
    }
    for (auto o : observers_) {
      o->ThreadStarted(threads_[i]->thread_id());
    }
  }
  return 0;
}

int IOThreadPool::Stop() {
  for (int i = 0; i < thread_num_; i++) {
    int ret = threads_[i]->Stop();
    if (ret != 0) {
      return ret;
    }
    for (auto o : observers_) {
      o->ThreadStoped(threads_[i]->thread_id());
    }
  }
  return 0;
}

void IOThreadPool::AddObserver(std::shared_ptr<Observer> o) {
  observers_.push_back(o);
}

void IOThreadPool::RemoveObserver(std::shared_ptr<Observer> o) {
  for (auto it = observers_.begin(); it != observers_.end(); it++) {
    if (*it == o) {
      observers_.erase(it);
      return;
    }
  }
}

std::shared_ptr<IOThread> IOThreadPool::NextThread() {
  return threads_[index_++ % thread_num_];
}

}  // namespace procyon
