#include "pink/iobuf.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

namespace pink {

IOBuf::IOBuf(const char* d) : IOBuf(d, strlen(d)) {}

IOBuf::IOBuf(const std::string& s) : IOBuf(s.data(), s.size()) {}

IOBuf::IOBuf(const char* d, size_t n) : IOBuf() {
  AllocateMem(n);
  memcpy(data_, d, n);
  length_ = n;
}

IOBuf::IOBuf()
    : data_(nullptr),
      length_(0),
      buffer_(nullptr),
      capacity_(0),
      own_buffer_(false),
      ref_count_(1),
      next_(this),
      prev_(this) {
}

IOBuf::~IOBuf() {
  while (next_ != this) {
    IOBuf* buf = next_;
    next_ = next_->next_;
    buf->Unref();
  }
  Unref();
}

void IOBuf::Ref() {
  ref_count_++;
}

void IOBuf::Unref() {
  ref_count_--;
  if (ref_count_ == 0) {
    if (own_buffer_) {
      delete buffer_;
    }
    delete this;
  }
}

std::unique_ptr<IOBuf> IOBuf::WrapBuffer(const char* src, size_t size) {
  std::unique_ptr<IOBuf> iobuf(new IOBuf);
  iobuf->data_ = const_cast<char*>(src);
  iobuf->length_ = size;
  iobuf->buffer_ = const_cast<char*>(src);
  iobuf->capacity_ = size;
  return std::move(iobuf);
}

std::unique_ptr<IOBuf> IOBuf::CopyBuffer(const char* src, size_t size) {
  std::unique_ptr<IOBuf> iobuf(new IOBuf(src, size));
  return std::move(iobuf);
}

size_t IOBuf::AllocateMem(size_t min_size) {
  min_size = (min_size + 7) & ~7;
  buffer_ = static_cast<char*>(malloc(min_size));
  capacity_ = min_size;
  data_ = buffer_;
  own_buffer_ = true;
  return min_size;
}

std::pair<char*, int> IOBufQueue::Allocate(size_t min_size) {
  IOBuf* iobuf = new IOBuf();
  size_t size = iobuf->AllocateMem(min_size);
  append(iobuf);
  return std::make_pair(iobuf->data(), size);
}

void IOBufQueue::append(const std::string& buf) {
  append(buf.data(), buf.size());
}

void IOBufQueue::append(const char* s) {
  append(s, strlen(s));
}

void IOBufQueue::append(const char* buf, size_t size) {
  IOBuf* iobuf = new IOBuf(buf, size);
  append(iobuf);
}

void IOBufQueue::append(IOBuf* buf) {
  assert(buf != nullptr);
  buf->Ref();
  if (!head_) {
    head_.reset(buf);
  } else {
    IOBuf* rear = head_->prev_;
    rear->next_->prev_ = buf;
    buf->prev_ = rear;
    buf->next_ = rear->next_;
    rear->next_ = buf;
  }
}

}  // namespace pink
