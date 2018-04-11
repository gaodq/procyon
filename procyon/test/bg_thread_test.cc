#include "procyon/bg_thread.h"

#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "gmock/gmock.h"

using ::testing::AtLeast;
using ::testing::Invoke;

std::mutex print_mu;

void TestFunc(void* arg) {
  std::lock_guard<std::mutex> l(print_mu);
  int num = *reinterpret_cast<int*>(arg);
  std::cout << num << '\n';
}

TEST(BGThreadTest, Normal) {
  procyon::BGThread bg_worker(4);
  ASSERT_EQ(bg_worker.Start(), 0);

  int* vec = new int[1000];
  for (int i = 0; i < 1000; i++) {
    vec[i] = i;
    bg_worker.Schedule(TestFunc, &vec[i]);
  }

  bg_worker.Join();
  delete[] vec;
}
