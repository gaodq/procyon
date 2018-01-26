#include <thread>

#include "pink/io_thread.h"

namespace pink {

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
  }
  return 0;
}

int IOThreadPool::Stop() {
  for (int i = 0; i < thread_num_; i++) {
    int ret = threads_[i]->Stop();
    if (ret != 0) {
      return ret;
    }
  }
  return 0;
}

std::shared_ptr<IOThread> IOThreadPool::NextThread() {
  return threads_[index_++ % thread_num_];
}

}  // namespace pink
