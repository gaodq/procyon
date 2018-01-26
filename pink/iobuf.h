#ifndef PINK_IOBUF_H_
#define PINK_IOBUF_H_

#include <pthread.h>
#include <string>
#include <memory>
#include <atomic>
#include <utility>

namespace pink {

class IOBuf {
 public:
  IOBuf(const char* d);
  IOBuf(const std::string& s);
  IOBuf(const char* d, size_t n);
  IOBuf();
  ~IOBuf();

  void Ref();
  void Unref();

  static std::unique_ptr<IOBuf> WrapBuffer(const char* src, size_t size);
  static std::unique_ptr<IOBuf> CopyBuffer(const char* src, size_t size);

  char* data() { return data_; }
  size_t length() { return length_; }

 private:
  friend class IOBufQueue;

  size_t AllocateMem(size_t min_size);

  char* data_;
  size_t length_;
  char* buffer_;
  size_t capacity_;

  bool own_buffer_;
  std::atomic<int> ref_count_;

  IOBuf* next_;
  IOBuf* prev_;
};

class IOBufQueue {
 public:
  IOBufQueue() = default;
  ~IOBufQueue();

  std::pair<char*, int> Allocate(size_t min_size);

  void append(const std::string& buf);
  void append(IOBuf* buf);
  void append(const char* str);
  void append(const char* buf, size_t size);

 private:
  std::unique_ptr<IOBuf> head_;
};

}  // namespace pink
#endif  // PINK_IOBUF_H_

