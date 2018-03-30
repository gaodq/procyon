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

  ASSERT_EQ(buf->ByteAt(0), 't');
  ASSERT_EQ(buf->ByteAt(8), 't');
  ASSERT_EQ(buf->ByteAt(9), 'e');

  ASSERT_EQ(buf->TEST_BlockCount(), 1);
  ASSERT_EQ(buf->ToString(), std::string(test_str));

  auto clone = buf->Split(5);
  auto pop = buf->Pop();
  ASSERT_EQ(pop->TEST_Refcount(), 2);

  delete buf;

  clone.reset();
  ASSERT_EQ(pop->TEST_Refcount(), 1);
}

TEST(IOBufTest, Buflen) {
  pink::IOBuf buf;
  std::string cmd("*1\r\n$7\r\nCOMMAND\r\n");
  buf.Append(cmd.data(), cmd.size());
  ASSERT_EQ(buf.length(), cmd.size());
  buf.Split(2);
  ASSERT_EQ(buf.length(), cmd.size() - 2);
  buf.TrimStart(2);
  ASSERT_EQ(buf.length(), cmd.size() - 4);
  buf.TrimEnd(2);
  ASSERT_EQ(buf.length(), cmd.size() - 6);
}

TEST(IOBufTest, MultiBlocks) {
}
