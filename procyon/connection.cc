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
  explicit IOHandler(Connection* c) : conn(c) {}

  void HandleReady(uint32_t events) override {
    if (events == kRead) {
      conn->PerformRead();
    } else if (events == kWrite) {
      conn->PerformWrite();
    } else if (events == kReadWrite) {
      conn->PerformWrite();
      conn->PerformRead();
    } else {
      conn->Close();
    }
  }

  Connection* conn;
};

Connection::Connection()
    : state_(kNoConnect),
      last_active_time_(std::chrono::system_clock::now()),
      io_handler_(new IOHandler(this)) {
}

void Connection::InitConn(int conn_fd, std::shared_ptr<IOThread> io_thread,
                          Dispatcher* dispacher, const EndPoint* remote_side,
                          const EndPoint* local_side) {
  conn_fd_ = conn_fd;
  io_thread_ = io_thread;
  dispatcher_ = dispacher;
  remote_side_ = *remote_side;
  local_side_ = *local_side;
  io_handler_->RegisterHandler(event_loop(), conn_fd_);
  state_ = kConnected;
}

/* TODO Support Client
bool Connection::Connect(const ClientOptions& opts,
                         const EndPoint* remote_side,
                         const EndPoint* local_side) {
  int rv;
  char cport[6];
  struct addrinfo hints, *servinfo, *p;
  snprintf(cport, sizeof(cport), "%d", remote_side->port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // Not support IPv6
  std::string ip_str;
  util::IPToStr(remote_side->ip, &ip_str);
  if ((rv = getaddrinfo(ip_str.c_str(), cport, &hints, &servinfo)) != 0) {
    log_err("getaddrinfo error: %s", strerror(errno));
    return false;
  }
  for (p = servinfo; p != nullptr; p = p->ai_next) {
    if ((conn_fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    // bind if needed
    if (local_side != nullptr) {
      struct sockaddr_in localaddr;
      localaddr.sin_family = AF_INET;
      localaddr.sin_addr = local_side->ip;
      localaddr.sin_port = local_side->port;  // Any local port will do
      if (bind(conn_fd_, (struct sockaddr *)&localaddr, sizeof(localaddr)) != 0) {
        log_err("bind error: %s", strerror(errno));
        return false;
      }
    }

    if (util::SetNonblocking(conn_fd_) != 0) {
      log_err("SetNonblocking error: %s", strerror(errno));
      return false;
    }

    if (connect(conn_fd_, p->ai_addr, p->ai_addrlen) == -1) {
      if (errno == EHOSTUNREACH) {
        close(conn_fd_);
        continue;
      } else if (errno == EINPROGRESS ||
                 errno == EAGAIN ||
                 errno == EWOULDBLOCK) {
        struct pollfd wfd[1];

        wfd[0].fd = conn_fd_;
        wfd[0].events = POLLOUT;

        int res;
        if ((res = poll(wfd, 1, opts.connect_timeout_ms)) == -1) {
          close(conn_fd_);
          freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH connect poll error: %s", strerror(errno));
          return false;
        } else if (res == 0) {
          close(conn_fd_);
          freeaddrinfo(servinfo);
          log_err("connect timeout: %s", strerror(errno));
          return false;
        }
        int val = 0;
        socklen_t lon = sizeof(int);

        if (getsockopt(conn_fd_, SOL_SOCKET, SO_ERROR, &val, &lon) == -1) {
          close(conn_fd_);
          freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH connect host getsockopt error: %s", strerror(errno));
          return false;
        }

        if (val) {
          close(conn_fd_);
          freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH connect host error: %s", strerror(errno));
          return false;
        }
      } else {
        close(conn_fd_);
        freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH cannot reach target host: %s", strerror(errno));
        return false;
      }
    }

    struct sockaddr_in laddr;
    socklen_t llen = sizeof(laddr);
    getsockname(conn_fd_, (struct sockaddr*) &laddr, &llen);
    std::string lip(inet_ntoa(laddr.sin_addr));
    int lport = ntohs(laddr.sin_port);
    if (ip_str == lip && remote_side->port == lport) {
      log_err("EHOSTUNREACH same ip port");
      return false;
    }

    // Set block
    int flags = fcntl(conn_fd_, F_GETFL, 0);
    fcntl(conn_fd_, F_SETFL, flags & ~O_NONBLOCK);
    freeaddrinfo(servinfo);

    // connect ok
    remote_side_ = *remote_side;
    last_active_time_ = std::chrono::system_clock::now();
    return true;
  }
  if (p == nullptr) {
    log_err("Can't create socket: %s", strerror(errno));
    return false;
  }
  freeaddrinfo(servinfo);
  freeaddrinfo(p);
  return false;
}
*/

void Connection::PerformRead() {
  last_active_time_ = std::chrono::system_clock::now();
  while (true) {
    void* buffer;
    size_t len;
    GetReadBuffer(&buffer, &len);
    ssize_t rn = read(conn_fd_, buffer, len);
    if (rn == 0) {
      Close();
      return;
    }
    if (rn < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      dispatcher_->OnConnError(this);
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
      Close();
      return;
    } else {
      std::string remain;
      if (sended < static_cast<ssize_t>(buf.size())) {
        remain.assign(buf.data() + sended + 1, buf.size() - sended);
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
      Close();
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

void Connection::Close(/* CLOSEREASON reason */) {
  event_loop()->RunInLoop([this](){
    if (state_ != kConnected) {
      return;
    }
    state_ = kNoConnect;
    dispatcher_->OnConnClosed(this);
    close(conn_fd_);
  });
}

}  // namespace procyon
