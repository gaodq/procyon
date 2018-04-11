#pragma once

#include <pthread.h>
#include <string>
#include <memory>

namespace procyon {

class Thread {
 public:
  class Runnable {
   public:
    virtual ~Runnable() {}
    virtual void run() = 0;

    Thread* thread() { return thread_; }
    void SetThread(Thread* t) { thread_ = t; }

   private:
    Thread* thread_;
  };

  Thread(std::shared_ptr<Runnable> runnable, bool detached = false);
  virtual ~Thread() {}

  enum STATE {kUninitialized, kStarted, kStopped};

  int Start();
  int Join(void** retval = nullptr);

  bool detached() { return detached_; }

  STATE state() { return state_; }

  pthread_t thread_id() const { return thread_id_; }

  const std::string& thread_name() const { return thread_name_; }

  void set_thread_name(const std::string& name) { thread_name_ = name; }

 private:
  static void* ThreadMain(void* arg);

  STATE state_;
  bool detached_;
  std::shared_ptr<Runnable> runnable_;
  pthread_t thread_id_;
  std::string thread_name_;

  // No allowed copy and copy assign
  Thread(const Thread&) = delete;
  void operator=(const Thread&) = delete;
};

}  // namespace procyon
