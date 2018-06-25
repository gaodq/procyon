#include "procyon/connection.h"

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>

#include "procyon/dispatcher.h"
#include "procyon/eventbase_loop.h"
#include "procyon/io_thread.h"
#include "procyon/xdebug.h"

namespace procyon {

struct Connection::IOHandler : public EventHandler {
  explicit IOHandler(ConnectionPtr c) : conn(c) {}

  void HandleReady(uint32_t events) override {
    if (events == kRead) {
      conn->PerformRead();
    } else if (events == kWrite) {
      conn->PerformWrite();
    } else if (events == kReadWrite) {
      conn->PerformWrite();
      conn->PerformRead();
    } else {
      log_warn("Connection %d closed on error", conn->fd());
      conn->CloseImpl();
    }
  }

  ConnectionPtr conn;
};

Connection::Connection()
    : state_(kNoConnect),
      last_active_time_(std::chrono::system_clock::now()) {
}

void Connection::InitConn(int conn_fd, std::shared_ptr<IOThread> io_thread,
                          Dispatcher* dispacher, const EndPoint* remote_side,
                          const EndPoint* local_side) {
  conn_fd_ = conn_fd;
  io_thread_ = io_thread;
  dispatcher_ = dispacher;
  remote_side_ = *remote_side;
  local_side_ = *local_side;
  io_handler_.reset(new IOHandler(shared_from_this()));
  io_handler_->RegisterHandler(event_loop(), conn_fd_);
  state_ = kConnected;
}

void Connection::PerformRead() {
  last_active_time_ = std::chrono::system_clock::now();
  while (true) {
    void* buffer;
    size_t len;
    GetReadBuffer(&buffer, &len);
    ssize_t rn = read(conn_fd_, buffer, len);
    if (rn == 0) {
      log_warn("Connection %d closed by peer", fd());
      CloseImpl();
      return;
    }
    if (rn < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      log_warn("Connection %d read error", fd());
      dispatcher_->OnConnError(conn_fd_);
      break;
    } else if (rn > 0) {
      OnDataAvailable(rn);
      if (rn < static_cast<ssize_t>(len)) {
        return;
      }
    }
  }
}

ssize_t Connection::WriteImpl(const char* data, size_t size) {
  assert(io_handler_);
  size_t sended = 0;
  while (sended < size) {
    ssize_t wn = write(conn_fd_, data + sended, size - sended);
    if (wn > 0) {
      sended += wn;
    } else if (wn == -1 && errno == EINTR) {
    } else if (wn == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      break;
    } else {
      return -1;
    }
  }
  return sended;
}

void Connection::PerformWrite() {
  assert(io_handler_);
  last_active_time_ = std::chrono::system_clock::now();

  io_handler_->DisableWrite();

  std::unique_lock<std::mutex> lk(pending_output_mu_);
  while (!pending_output_.empty()) {
    WriteRequest& req = pending_output_.front();
    const std::string& buf = req.data;
    std::promise<bool> res = std::move(req.res);
    lk.unlock();

    ssize_t sended = WriteImpl(buf.data(), buf.size());
    if (sended < 0) {
      // Error
      res.set_value(false);
      log_warn("Connection %d closed while PerformWrite error", fd());
      CloseImpl();
      return;
    } else {
      std::string remain;
      if (sended < static_cast<ssize_t>(buf.size())) {
        remain.assign(buf.data() + sended, buf.size() - sended);
        io_handler_->EnableWrite();
      } else {
        assert(sended == static_cast<ssize_t>(buf.size()));
        res.set_value(true);
      }
      lk.lock();
      pending_output_.pop_front();
      if (!remain.empty()) {  // New pending data
        pending_output_.emplace_front(WriteRequest{std::move(remain), std::move(res)});
        return;
      }
    }
  }
}

std::future<bool> Connection::Write(const void* data, size_t size) {
  assert(io_handler_);
  last_active_time_ = std::chrono::system_clock::now();
  const char* buf = reinterpret_cast<const char*>(data);
  std::promise<bool> res;
  std::future<bool> wait_complete = res.get_future();

  if (state_ != kConnected) {
    res.set_value(false);
    return wait_complete;
  }

  std::unique_lock<std::mutex> lk(pending_output_mu_);
  if (!pending_output_.empty()) {
    pending_output_.emplace_back(
      WriteRequest{std::string(buf, size), std::move(res)});
    io_handler_->EnableWrite();
  } else {
    pending_output_mu_.unlock();
    ssize_t sended = WriteImpl(buf, size);

    if (sended < 0) {
      // Error
      res.set_value(false);
      log_warn("Connection %d closed while Write error", fd());
      CloseImpl();
    } else if (sended < static_cast<ssize_t>(size)) {
      pending_output_mu_.lock();
      pending_output_.emplace_back(
        WriteRequest{std::string(buf + sended, size - sended), std::move(res)});
      io_handler_->EnableWrite();
    } else {
      assert(sended == static_cast<ssize_t>(size));
      // Success write all data
      res.set_value(true);
    }
  }

  return wait_complete;
}

int Connection::IdleSeconds() {
  auto diff = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now() - last_active_time_);
  return diff.count();
}

void Connection::CloseImpl() {
  assert(io_handler_);
  if (state_ != kConnected) {
    return;
  }

  state_ = kNoConnect;

  {
  std::unique_lock<std::mutex> lk(pending_output_mu_);
  while (!pending_output_.empty()) {
    WriteRequest& req = pending_output_.front();
    req.res.set_value(false);
    pending_output_.pop_front();
  }
  }

  io_handler_->UnRegisterHandler();
  close(conn_fd_);
  dispatcher_->OnConnClosed(conn_fd_);
}

void Connection::Close(/* CLOSEREASON reason */) {
  event_loop()->RunInLoop([this](){ CloseImpl(); });
}

}  // namespace procyon
