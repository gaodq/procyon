#pragma once

#include <string>

#include <netinet/in.h>
#include <sys/socket.h>

namespace pink {

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

bool StrToEndPoint(const std::string& ip_port, EndPoint* ep);
bool EndPointToStr(const EndPoint& ep, std::string* ip_port);

bool StrToIP(const char* ip_str, ip_t* ip);
bool StrToIP(const std::string& ip_str, ip_t* ip);

bool IPToStr(ip_t ip, std::string* ip_str);

bool HostnameToIP(const char* hostname, ip_t* ip);
bool HostnameToIP(const std::string& hostname, ip_t* ip);

bool HostnameToEndPoint(const char* hostname, int port, EndPoint* ep);
bool HostnameToEndPoint(const std::string hostname, int port, EndPoint* ep);

};  // namespace pink
