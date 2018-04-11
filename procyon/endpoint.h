#pragma once

#include <string>

#include <netinet/in.h>
#include <sys/socket.h>

namespace procyon {

typedef struct in_addr ip_t;

struct EndPoint {
  EndPoint() : ip({INADDR_ANY}), port(0) {}
  EndPoint(ip_t i, int p) : ip(i), port(p) {}
  EndPoint(int p) : ip({INADDR_ANY}), port(p) {}
  explicit EndPoint(const sockaddr_in& addr)
      : ip(addr.sin_addr), port(addr.sin_port) {}

  ip_t ip;
  int port;
};

};  // namespace procyon
