#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <string>
#include <functional>

namespace pink {

class ServerSocket {
 public:
  ServerSocket();
  ~ServerSocket();

  int fd() { return fd_; }

  int Bind(const std::string &ip, int port);
  int Listen();

 private:
  int fd_;

  int send_timeout_;
  int recv_timeout_;
  int accept_timeout_;
  int accept_backlog_;
  int tcp_send_buffer_;
  int tcp_recv_buffer_;
};

}  // namespace pink
