#include "pink/connection.h"

#include <unistd.h>
#include <stdio.h>

#include "src/dispatcher.h"
#include "src/eventbase_loop.h"
#include "src/io_thread.h"

namespace pink {

struct Connection::IoHandler : public EventHandler {
  explicit IoHandler(Connection* c) : conn(c) {}

  void HandleReady(uint32_t events) override {
    if (events == kRead) {
      conn->PerformRead();
    } else if (events == kWrite) {
      conn->PerformWrite();
    } else if (events == kReadWrite) {
      conn->PerformWrite();
    } else {
      conn->Close();
    }
  }

  Connection* conn;
};

Connection::Connection()
    : buf_pos_(0),
      io_handler_(new IoHandler(this)) {
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
    ssize_t rn = read(fd_, read_buf_ + buf_pos_, 4096 - buf_pos_);
    if (rn < 0) {
      if (errno == EAGAIN) {
        return;
      }
      dispatcher_->error_cb_(this);
      break;
    } else if (rn > 0) {
      bool r = OnDataAvailable(rn);
      if (!r) {
        dispatcher_->error_cb_(this);
        break;
      }
    } else {
      // EOF
      break;
    }
  }
  Close();
}

void Connection::PerformWrite() {
}

int Connection::Write(const char* msg, size_t size) {
  ssize_t wn = write(fd_, msg, size);

  return wn;
}

void Connection::Close(/* CLOSEREASON reason */) {
  // Connection closed by peer
  dispatcher_->close_cb_(this);
}

}  // namespace pink
