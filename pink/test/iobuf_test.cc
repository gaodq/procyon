#include "pink/iobuf.h"

#include <memory>
#include <string.h>

#include "gmock/gmock.h"

using ::testing::AtLeast;
using ::testing::Invoke;

TEST(IOBufTest, SingleBlock) {
  pink::IOBuf* buf = new pink::IOBuf();

  auto mem = buf->PreAllocate();

  const char* test_str = "this is test text";
  memcpy(mem.first, test_str, strlen(test_str));
  buf->PostAllocate(strlen(test_str));

  ASSERT_EQ(buf->ByteAt(0), 't');
  ASSERT_EQ(buf->ByteAt(8), 't');
  ASSERT_EQ(buf->ByteAt(9), 'e');

  ASSERT_EQ(buf->TEST_BlockCount(), 1);
  ASSERT_EQ(buf->ToString(), std::string(test_str));

  auto clone = buf->Split(5);
  // // auto pop = buf->Pop();
  // ASSERT_EQ(pop->TEST_Refcount(), 2);

  // clone.reset();
  // ASSERT_EQ(pop->TEST_Refcount(), 1);

  delete buf;
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
  pink::IOBuf buf;
  auto mem = buf.PreAllocate();
  ASSERT_EQ(mem.second, 16380);
  buf.PostAllocate(mem.second);

  mem = buf.PreAllocate();
  *(char*)mem.first = 'c';
  *((char*)mem.first + 16379) = 'a';
  ASSERT_EQ(mem.second, 16380);
  buf.PostAllocate(mem.second);
  ASSERT_EQ(buf.TEST_BlockCount(), 2);

  mem = buf.PreAllocate();
  ASSERT_EQ(mem.second, 16380);
  buf.PostAllocate(mem.second);
  ASSERT_EQ(buf.TEST_BlockCount(), 3);

  buf.TrimStart(16000);
  buf.TrimStart(380);
  ASSERT_EQ(buf.TEST_BlockCount(), 2);
  ASSERT_EQ(buf.ByteAt(0), 'c');

  buf.Split(380);
  buf.Split(15999);
  ASSERT_EQ(buf.ByteAt(0), 'a');

  mem = buf.PreAllocate();
  buf.PostAllocate(mem.second);
  *((char*)mem.first + 391) = 'b';

  buf.TrimStart(16380 + 1 + 390);
  ASSERT_EQ(buf.ByteAt(1), 'b');
  ASSERT_EQ(buf.TEST_BlockCount(), 1);
}
