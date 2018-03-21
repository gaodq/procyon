#include "pink/connection.h"

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

#include "pink/dispatcher.h"
#include "pink/eventbase_loop.h"
#include "pink/io_thread.h"
#include "pink/util.h"
#include "pink/xdebug.h"

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

void Connection::InitConn(int conn_fd, std::shared_ptr<IOThread> io_thread,
                          Dispatcher* dispacher, const EndPoint* remote_side,
                          const EndPoint* local_side) {
  conn_fd_ = conn_fd;
  io_thread_ = io_thread;
  dispatcher_ = dispacher;
  remote_side_ = *remote_side;
  local_side_ = *local_side;
  io_handler_->RegisterHandler(io_thread_->event_loop(), conn_fd_);
}

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

void Connection::PerformRead() {
  while (true) {
    void* buffer;
    size_t len;
    GetReadBuffer(&buffer, &len);
    ssize_t rn = read(conn_fd_, buffer, len);
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

ssize_t Connection::WriteImpl(const char* data, size_t size) {
  assert(io_handler_);
  size_t sended = 0;
  while (sended < size) {
    ssize_t wn = write(conn_fd_, data + sended, size - sended);
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
        break;
      }
    }
  }
  if (pending_output_.empty()) {
  }
}

bool Connection::Write(const void* data, size_t size) {
  assert(io_handler_);
  const char* buf = reinterpret_cast<const char*>(data);
  if (!pending_output_.empty()) {
    pending_output_.emplace_back(std::string(buf, size));
    io_handler_->EnableWrite();
  } else {
    ssize_t sended = WriteImpl(buf, size);

    if (sended < 0) {
      return false;
    } else if (sended < static_cast<ssize_t>(size)) {
      pending_output_.emplace_back(std::string(buf + sended, size - sended));
    }
  }

  return true;
}

bool Connection::BlockWrite(const void* data, size_t size) {
  size_t sended = 0;
  const char* buf = reinterpret_cast<const char*>(data);
  while (sended < size) {
    ssize_t wn = write(conn_fd_, buf + sended, size - sended);
    if (wn > 0) {
      sended += wn;
    } else if (wn == -1 && errno == EINTR) {
    } else if (wn == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // Timeout
      log_err("send timeout: %s", strerror(errno));
      return false;
    } else {
      // IOError
      log_err("IOError: %s", strerror(errno));
      return false;
    }
  }
  return true;
}

bool Connection::BlockRead(void* data, size_t buf_size, size_t* received) {
  char* rbuf = reinterpret_cast<char*>(data);
  size_t nleft = buf_size;
  size_t pos = 0;
  ssize_t nread;

  while (nleft > 0) {
    if ((nread = read(conn_fd_, rbuf + pos, nleft)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        log_err("send timeout: %s", strerror(errno));
        return false;
      } else {
        log_err("read error: %s", strerror(errno));
        return false;
      }
    } else if (nread == 0) {
      log_err("socket closed");
      return false;
    }
    nleft -= nread;
    pos += nread;
  }

  *received = pos;
  return true;
}

void Connection::Close(/* CLOSEREASON reason */) {
  // Connection closed by peer
  dispatcher_->OnConnClosed(this);
}

}  // namespace pink
