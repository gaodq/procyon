#pragma once

#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "procyon/thread.h"

namespace procyon {

typedef void (*Func)(void*);


class BGThread {
 public:
  class Worker : public Thread::Runnable {
   public:
    explicit Worker(BGThread* bt) : bg_thread_(bt) {}
    virtual void run() override;

   private:
    BGThread* const bg_thread_;
  };

  explicit BGThread(size_t thread_num = 1, size_t max_queue = 100000);

  virtual ~BGThread() { Stop(); }

  int Start();
  void Stop();
  int Join();

  void Schedule(Func fn, void* arg);
  void DelaySchedule(uint64_t timeout_ms, Func fn, void* arg);

  void QueueSize(int* pri_size, int* qu_size);
  void QueueClear();

 private:
  struct BGItem {
    BGItem(Func fn, void* _arg) : func(fn), arg(_arg) {}
    Func func;
    void* arg;
  };

  struct TimerItem {
    TimerItem(uint64_t _exec_time, Func fn, void* _arg)
        : exec_time(_exec_time),
          func(fn),
          arg(_arg) {
    }
    uint64_t exec_time;
    Func func;
    void* arg;
  
    bool operator < (const TimerItem& item) const {
      return exec_time > item.exec_time;
    }
  };

  std::queue<BGItem> queue_;
  std::priority_queue<TimerItem> timer_queue_;

  volatile bool should_stop_;

  size_t thread_num_;
  size_t max_queue_;

  std::mutex mu_;
  std::condition_variable rsignal_;
  std::condition_variable wsignal_;

  std::vector<std::unique_ptr<Thread>> thread_list_;
};

}  // namespace procyon
