#include "iobuf.h"

#include <string.h>

namespace pink {

Block::Block(size_t capacity)
      : next_(this),
        prev_(this) {
  size_t minsize = capacity + sizeof(SharedInfo);
  size_t goodsize = (minsize + 7) & ~7;
  buffer_ = static_cast<char*>(malloc(goodsize));
  data_ = buffer_;
  length_ = 0;
  capacity_ = goodsize;
  char* info_start = buffer_ + capacity_ - sizeof(SharedInfo);
  SharedInfo* info = new(info_start) SharedInfo;
  info->store(1);
}

Block::Block(const Block& b) {
  std::unique_ptr<Block> cb = b.Clone();
  *this = *cb.release();
}

Block::Block(char* buf, char* data, size_t len, size_t cap)
    : buffer_(buf),
      data_(data),
      length_(len),
      capacity_(cap),
      next_(this),
      prev_(this) {
}

Block::~Block() {
  Unref();
}

void Block::Ref() {
  shared_info()->fetch_add(1);
}

void Block::Unref() {
  int ref = shared_info()->fetch_sub(1);
  if (ref > 1) {
    return;
  }
  free(buffer_);
}

void Block::TrimStart(size_t amount) {
  assert(amount <= length_);
  data_ += amount;
  length_ -= amount;
}

void Block::TrimEnd(size_t amount) {
  assert(amount <= length_);
  length_ -= amount;
}

std::unique_ptr<Block> Block::Clone() const {
  Block* b = new Block(buffer_, data_, length_, capacity_);
  b->Ref();
  return std::move(std::unique_ptr<Block>(b));
}

IOBuf::~IOBuf() {
  while (head_) {
    Pop();
  }
}

std::pair<void*, size_t> IOBuf::PreAllocate(size_t size) {
  size_t alloc_size = std::max(size, kDefaultBufferSize);
  std::pair<void*, size_t> res;

  if (!head_) {
    head_.reset(new Block(alloc_size));
    res.first = head_->buffer_;
    res.second = head_->capacity_;
  } else if (head_->prev_->Space() > 0) {
    res.first = head_->End();
    res.second = head_->Space();
  } else {
    std::unique_ptr<Block> nb(new Block(alloc_size));
    res.first = nb->buffer_;
    res.second = nb->capacity_;
    Append(std::move(nb));
  }

  return res;
}

void IOBuf::PostAllocate(size_t size) {
  assert(head_);
  head_->prev_->Append(size);
}

void IOBuf::Append(std::unique_ptr<Block>&& block) {
  Block* b = block.release();
  length_ += b->length_;
  if (!head_) {
    head_.reset(b);
    return;
  }

  Block* rear = head_->prev_;

  rear->next_->prev_ = b;
  b->next_ = rear->next_;
  b->prev_ = rear;
  rear->next_ = b;
}

void IOBuf::Append(const char* data, size_t size) {
  int remain = size;
  while (remain) {
    auto mem = PreAllocate(remain);
    memcpy(mem.first, data, mem.second);
    remain -= mem.second;
    PostAllocate(mem.second);
  }
  length_ += size;
}

std::unique_ptr<Block> IOBuf::Pop() {
  if (!head_) {
    return std::move(head_);
  }

  length_ -= head_->length_;

  if (head_->prev_ == head_.get()) {
    return std::move(head_);
  }

  Block* rear = head_->prev_;
  Block* new_head = head_->next_;
  rear->next_ = new_head;
  new_head->prev_ = rear;

  std::unique_ptr<Block> nh(new_head);
  head_.swap(nh);
  return nh;
}

void IOBuf::TrimStart(size_t amount) {
  while (amount > 0) {
    if (!head_) {
      break;
    }
    if (head_->length_ > amount) {
      head_->TrimStart(amount);
      amount = 0;
      length_ -= amount;
      break;
    }
    amount -= head_->length_;
    length_ -= head_->length_;
    Pop();
  }
}

void IOBuf::TrimEnd(size_t amount) {
  while (amount > 0) {
    if (!head_) {
      break;
    }
    if (head_->prev_->length_ > amount) {
      head_->prev_->TrimEnd(amount);
      length_ -= amount;
      amount = 0;
      break;
    }
    amount -= head_->prev_->length_;
    length_ -= head_->prev_->length_;
    // Remove rear
    Block* rear = head_->prev_;
    if (rear == head_.get()) {
      head_.reset();
    } else {
      rear->prev_->next_ = rear->next_;
      rear->next_->prev_ = rear->prev_;
      rear->Unref();
    }
  }
}

std::unique_ptr<IOBuf> IOBuf::Split(size_t n) {
  IOBuf* buf = new IOBuf();

  while (n > 0) {
    if (!head_) {
      break;
    } else if (head_->length_ <= n) {
      n -= head_->length_;
      length_ -= head_->length_;
      buf->Append(std::move(Pop()));
    } else {
      std::unique_ptr<Block> b = head_->Clone();
      b->TrimEnd(b->length_ - n);
      buf->Append(std::move(b));
      head_->TrimStart(n);
      length_ -= n;
      break;
    }
  }

  return std::unique_ptr<IOBuf>(buf);
}

std::string IOBuf::ToString() {
  std::string buf;
  Block* b = head_.get();
  while (b) {
    buf.append(b->ToString());

    if (b->next_ == head_.get()) {
      break;
    }
    b = b->next_;
  }
  return buf;
}

size_t IOBuf::TEST_BlockCount() {
  size_t count = 0;
  if (head_) {
    count = 1;
    Block* b = head_->next_;
    while (b != head_.get()) {
      count++;
      b = b->next_;
    }
  }
  return count;
}

};  // namespace pink
