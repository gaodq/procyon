#include "pink/connection.h"

#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "pink/dispatcher.h"
#include "pink/eventbase_loop.h"
#include "pink/io_thread.h"

namespace pink {

struct Connection::IOHandler : public EventHandler {
  explicit IOHandler(Connection* c) : conn(c) {}

  void HandleReady(uint32_t events) override {
    if (events == kRead) {
      conn->PerformRead();
    } else if (events == kWrite ||
               events == kReadWrite) {
      conn->PerformWrite();
    } else {
      conn->Close();
    }
  }

  Connection* conn;
};

Connection::Connection()
    : io_handler_(new IOHandler(this)) {
}

bool Connection::InitConnection(int fd, std::shared_ptr<IOThread> io_thread,
                                Dispatcher* dispatcher) {
  fd_ = fd;
  io_thread_ = io_thread;
  dispatcher_ = dispatcher;

  return io_handler_->RegisterHandler(io_thread_->event_loop(), fd_);
}

void Connection::PerformRead() {
  while (true) {
    void* buffer;
    size_t len;
    GetReadBuffer(&buffer, &len);
    ssize_t rn = read(fd_, buffer, len);
    if (rn < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      dispatcher_->OnConnError(this);
      break;
    } else if (rn > 0) {
      bool r = OnDataAvailable(rn);
      if (!r) {
        dispatcher_->OnConnError(this);
        break;
      }
    } else {
      // EOF
      break;
    }
  }
  Close();
}

ssize_t Connection::WriteImpl(const char* msg, size_t size) {
  size_t sended = 0;
  while (sended < size) {
    ssize_t wn = write(fd_, msg + sended, size - sended);
    if (wn > 0) {
      sended += wn;
    } else if (wn == -1 && errno == EINTR) {
    } else if (wn == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      io_handler_->EnableWrite();
      break;
    } else {
      return -1;
    }
  }
  return sended;
}

void Connection::PerformWrite() {
  assert(!pending_output_.empty());
  while (!pending_output_.empty()) {
    const std::string& buf = pending_output_.front();
    ssize_t sended = WriteImpl(buf.data(), buf.size());
    if (sended < 0) {
      // Error
    } else {
      std::string remain;
      if (sended < static_cast<ssize_t>(buf.size())) {
        remain.assign(buf.data() + sended + 1, buf.size() - sended - 1);
      }
      pending_output_.pop_front();
      if (!remain.empty()) {
        pending_output_.emplace_front(remain);
      }
    }
  }
}

bool Connection::Write(const char* msg, size_t size) {
  if (!pending_output_.empty()) {
    pending_output_.emplace_back(std::string(msg, size));
    io_handler_->EnableWrite();
  } else {
    ssize_t sended = WriteImpl(msg, size);

    if (sended < 0) {
      return false;
    } else if (sended < static_cast<ssize_t>(size)) {
      pending_output_.emplace_back(std::string(msg + sended, size - sended));
    }
  }

  return true;
}

void Connection::Close(/* CLOSEREASON reason */) {
  // Connection closed by peer
  dispatcher_->OnConnClosed(this);
}

}  // namespace pink
