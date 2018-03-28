#include "pink/iobuf.h"

#include "string.h"

#include "gmock/gmock.h"

using ::testing::AtLeast;
using ::testing::Invoke;

TEST(IOBufTest, Normal) {
  pink::IOBuf buf;

  const size_t _1M = 1024 * 1024;
  auto mem = buf.PreAllocate(_1M);
  ASSERT_EQ(mem.second, _1M + 8);
  buf.PostAllocate(_1M + 8);
  mem = buf.PreAllocate(_1M - 8);
  ASSERT_EQ(mem.second, _1M);
  buf.PostAllocate(_1M);

  ASSERT_EQ(buf.TEST_BlockCount(), 2);
}
