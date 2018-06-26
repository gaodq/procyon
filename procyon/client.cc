#include "procyon/client.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>

#include "procyon/xdebug.h"
#include "procyon/io_thread.h"
#include "procyon/util.h"

namespace procyon {

Client::Client(const ClientOptions& options)
    : options_(options),
      conn_(options_.conn_factory->NewConnection()){
}

bool Client::Connect() {
  int rv, conn_fd;
  char cport[6];
  struct addrinfo hints, *servinfo, *p;
  snprintf(cport, sizeof(cport), "%d", options_.remote_port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  // Not support IPv6
  if ((rv = getaddrinfo(options_.remote_ip.c_str(), cport, &hints, &servinfo)) != 0) {
    log_err("getaddrinfo error: %s", strerror(errno));
    return false;
  }
  for (p = servinfo; p != nullptr; p = p->ai_next) {
    if ((conn_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }
    // bind if needed
    if (!options_.local_ip.empty()) {
      struct sockaddr_in localaddr;
      localaddr.sin_family = AF_INET;
      localaddr.sin_addr.s_addr = inet_addr(options_.local_ip.c_str());
      localaddr.sin_port = 0;  // Any local port will do
      if (bind(conn_fd, (struct sockaddr *)&localaddr, sizeof(localaddr)) != 0) {
        log_err("bind error: %s", strerror(errno));
        return false;
      }
    }
    if (util::SetNonblocking(conn_fd) != 0) {
      log_err("SetNonblocking error: %s", strerror(errno));
      return false;
    }
    if (connect(conn_fd, p->ai_addr, p->ai_addrlen) == -1) {
      if (errno == EHOSTUNREACH) {
        close(conn_fd);
        continue;
      } else if (errno == EINPROGRESS ||
                 errno == EAGAIN ||
                 errno == EWOULDBLOCK) {
        struct pollfd wfd[1];
        wfd[0].fd = conn_fd;
        wfd[0].events = POLLOUT;
        int res;
        if ((res = poll(wfd, 1, options_.connect_timeout_ms)) == -1) {
          close(conn_fd);
          freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH connect poll error: %s", strerror(errno));
          return false;
        } else if (res == 0) {
          close(conn_fd);
          freeaddrinfo(servinfo);
          log_err("connect timeout: %s", strerror(errno));
          return false;
        }
        int val = 0;
        socklen_t lon = sizeof(int);
        if (getsockopt(conn_fd, SOL_SOCKET, SO_ERROR, &val, &lon) == -1) {
          close(conn_fd);
          freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH connect host getsockopt error: %s", strerror(errno));
          return false;
        }
        if (val) {
          close(conn_fd);
          freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH connect host error: %s", strerror(errno));
          return false;
        }
      } else {
        close(conn_fd);
        freeaddrinfo(servinfo);
          log_err("EHOSTUNREACH cannot reach target host: %s", strerror(errno));
        return false;
      }
    }
    struct sockaddr_in laddr;
    socklen_t llen = sizeof(laddr);
    getsockname(conn_fd, (struct sockaddr*) &laddr, &llen);
    std::string lip(inet_ntoa(laddr.sin_addr));
    int lport = ntohs(laddr.sin_port);
    if (options_.remote_ip == lip && options_.remote_port == lport) {
      log_err("EHOSTUNREACH same ip port");
      return false;
    }
    // Set block
    int flags = fcntl(conn_fd, F_GETFL, 0);
    fcntl(conn_fd, F_SETFL, flags & ~O_NONBLOCK);
    freeaddrinfo(servinfo);

    // connect ok
    EndPoint local_side(laddr);
    EndPoint remote_side;
    util::IPPortToEndPoint(options_.remote_ip, options_.remote_port, &remote_side);
    conn_->InitConn(conn_fd, options_.io_thread, this, remote_side, local_side);

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

void Client::OnConnClosed(int) {
  if (options_.close_callback) {
    options_.close_callback(conn_);
  }
}

void Client::OnConnError(int) {
  if (options_.error_callback) {
    options_.error_callback(conn_);
  }
}

}  // namespace procyon
