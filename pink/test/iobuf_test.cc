#include "pink/iobuf.h"

#include <memory>
#include <string.h>

#include "gmock/gmock.h"

using ::testing::AtLeast;
using ::testing::Invoke;

TEST(IOBufTest, SingleBlock) {
  pink::IOBuf* buf = new pink::IOBuf();

  const size_t _1M = 1024 * 1024;
  auto mem = buf->PreAllocate(_1M);

  const char* test_str = "this is test text";
  memcpy(mem.first, test_str, strlen(test_str));
  buf->PostAllocate(strlen(test_str));

  ASSERT_EQ(buf->TEST_BlockCount(), 1);
  ASSERT_EQ(buf->ToString(), std::string(test_str));

  auto clone = buf->Split(5);
  auto pop = buf->Pop();
  ASSERT_EQ(pop->TEST_Refcount(), 2);

  delete buf;

  clone.reset();
  ASSERT_EQ(pop->TEST_Refcount(), 1);
}

TEST(IOBufTest, MultiBlocks) {
}
