#include "pink/server_socket.h"

#include <map>

#include "pink/xdebug.h"
#include "pink/util.h"

namespace pink {

ServerSocket::ServerSocket()
    : send_timeout_(0),
      recv_timeout_(0),
      accept_timeout_(0),
      accept_backlog_(1024),
      tcp_send_buffer_(0),
      tcp_recv_buffer_(0) {
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
}

ServerSocket::~ServerSocket() {
  close(fd_);
}

int ServerSocket::Bind(const std::string &ip, int port) {
  struct sockaddr_in addr;
  int ret = 0;
  memset(&addr, 0, sizeof(addr));

  if (util::SetReuseAddr(fd_) < 0) {
    log_err("Set REUSEADDR failed!");
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr =
    ip.empty() ? htonl(INADDR_ANY) : inet_addr(ip.c_str());
  addr.sin_port = htons(port);

  // TODO
  // local_side_.ip = addr.sin_addr.s_addr;
  // local_side_.port = port;

  util::SetFdCloseExec(fd_);

  ret = bind(fd_, (struct sockaddr *) &addr, sizeof(addr));
  if (ret < 0) {
    log_err("Bind fd_:%d failed", fd_);
    return -1;
  }
  return 0;
}

/*
 * Listen to a specific ip addr on a multi eth machine
 * Return 0 if Listen success, other wise
 */
int ServerSocket::Listen() {
  int ret = listen(fd_, accept_backlog_);
  if (ret < 0) {
    log_err("Listen fd_: %d failed!", fd_);
    return -1;
  }
  util::SetNonblocking(fd_);
  return 0;
}

}  // namespace pink
