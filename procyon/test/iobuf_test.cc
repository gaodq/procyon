#include "procyon/iobuf.h"

#include <memory>
#include <string.h>

#include "gmock/gmock.h"

using ::testing::AtLeast;
using ::testing::Invoke;

TEST(IOBufTest, SingleBlock) {
  procyon::IOBuf* buf = new procyon::IOBuf();

  auto mem = buf->PreAllocate();

  const char* test_str = "this is test text";
  memcpy(mem.first, test_str, strlen(test_str));
  buf->PostAllocate(strlen(test_str));

  ASSERT_EQ(buf->ByteAt(0), 't');
  ASSERT_EQ(buf->ByteAt(8), 't');
  ASSERT_EQ(buf->ByteAt(9), 'e');

  ASSERT_EQ(buf->TEST_BlockCount(), 1);
  ASSERT_EQ(buf->ToString(), std::string(test_str));

  delete buf;
}

TEST(IOBufTest, Buflen) {
  procyon::IOBuf buf;
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
  procyon::IOBuf buf;
  auto mem = buf.PreAllocate();
  ASSERT_EQ(mem.second, 16380);
  *(char*)mem.first = 'c';
  *((char*)mem.first + mem.second - 1) = 'a';
  buf.PostAllocate(mem.second);

  // mem = buf.PreAllocate();
  // *(char*)mem.first = 'c';
  // *((char*)mem.first + mem.second - 1) = 'a';
  // ASSERT_EQ(mem.second, 16380);
  // buf.PostAllocate(mem.second);
  // ASSERT_EQ(buf.TEST_BlockCount(), 2);

  // mem = buf.PreAllocate();
  // ASSERT_EQ(mem.second, 16380);
  // buf.PostAllocate(mem.second);
  // ASSERT_EQ(buf.TEST_BlockCount(), 3);

  // buf.TrimStart(16000);
  // buf.TrimStart(380);
  // ASSERT_EQ(buf.TEST_BlockCount(), 2);
  // ASSERT_EQ(buf.ByteAt(0), 'c');

  // buf.Split(380);
  // buf.Split(15999);
  // ASSERT_EQ(buf.ByteAt(0), 'a');

  // mem = buf.PreAllocate();
  // buf.PostAllocate(mem.second);
  // *((char*)mem.first + 391) = 'b';

  // buf.TrimStart(16380 + 1 + 390);
  // ASSERT_EQ(buf.ByteAt(1), 'b');
  // ASSERT_EQ(buf.TEST_BlockCount(), 1);
}

TEST(IOBufTest, Roll) {
  procyon::IOBuf buf;
  int count = 1000;
  while (count--) {
    auto mem = buf.PreAllocate();
    buf.PostAllocate(mem.second / 2);
    ASSERT_EQ(buf.length(), mem.second / 2);

    size_t remain = mem.second / 2;
    while (remain) {
      size_t min_sz = std::min(remain, static_cast<size_t>(1024));
      buf.TrimStart(min_sz);
      remain -= min_sz;
    }
    ASSERT_EQ(buf.length(), 0);
  }
}
