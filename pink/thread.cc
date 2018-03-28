#include "pink/thread.h"

#include "pink/util.h"
#include "pink/xdebug.h"

namespace pink {

Thread::Thread(std::shared_ptr<Runnable> runnable, bool detached)
    : state_(kUninitialized),
      detached_(detached),
      runnable_(runnable),
      thread_id_(0) {
  runnable_->SetThread(this);
}

void* Thread::ThreadMain(void *arg) {
  Thread* thread = reinterpret_cast<Thread*>(arg);
  if (!(thread->thread_name().empty())) {
    util::SetThreadName(pthread_self(), thread->thread_name());
  }
  thread->state_ = kStarted;
  thread->runnable_->run();
  thread->state_ = thread->detached_ ? kUninitialized : kStopped;
  return nullptr;
}

int Thread::Start() {
  if (state_ != kUninitialized) {
    log_warn("Thread has started");
    return -1;
  }
  if (pthread_create(&thread_id_, nullptr, ThreadMain, (void *)this) != 0) {
    log_err("pthread_create failed!");
    return -1;
  }
  if (detached_ && pthread_detach(thread_id_) != 0) {
    log_err("pthread_detach failed!");
    return -1;
  }
  return 0;
}

int Thread::Join(void** retval) {
  if (detached_) {
    log_info("Thread has been detached");
    return 0;
  }
  detached_ = true;
  state_ = kUninitialized;
  return pthread_join(thread_id_, retval);
}

}  // namespace pink
