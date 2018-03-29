#pragma once

#include <assert.h>
#include <memory>
#include <atomic>
#include <utility>

namespace pink {

class Block {
 public:
  typedef std::atomic<int> SharedInfo;

  Block(size_t capacity);
  Block(const Block& b);
  ~Block();

  void Ref();
  void Unref();

  void TrimStart(size_t amount);
  void TrimEnd(size_t amount);

  std::unique_ptr<Block> Clone() const;

  void Append(size_t size) {
    assert(length_ + size <= capacity_);
    length_ += size;
  }

  size_t Space() { return capacity_ - length_; }

  char* End() { return buffer_ + length_; }

  std::string ToString() { return std::string(data_, length_); }

  int TEST_Refcount() { return shared_info()->load(); }

 private:
  friend class IOBuf;
  Block(char* buf, char* data, size_t len, size_t cap);

  SharedInfo* shared_info() {
    return reinterpret_cast<SharedInfo*>(
      buffer_ + capacity_ - sizeof(SharedInfo));
  }

  char* buffer_;
  char* data_;
  size_t length_;
  size_t capacity_;

  Block* next_;
  Block* prev_;
};

class IOBuf {
  const size_t kDefaultBufferSize = 16 * 1024;

 public:
  IOBuf() : length_(0) {}
  ~IOBuf();

  std::pair<void*, size_t> PreAllocate(size_t size);
  void PostAllocate(size_t size);

  size_t length() { return length_; }

  void Append(std::unique_ptr<Block>&& block);
  void Append(const char* data, size_t size);
  std::unique_ptr<Block> Pop();

  void TrimStart(size_t amount);
  void TrimEnd(size_t amount);

  std::unique_ptr<IOBuf> Split(size_t n);

  std::string ToString();

  size_t TEST_BlockCount();

 private:
  size_t length_;
  std::unique_ptr<Block> head_;
};

};  // namespace pink
