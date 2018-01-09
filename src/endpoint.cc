#include "pink/endpoint.h"

#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <cstdlib>

namespace pink {

bool StrToEndPoint(const std::string& ip_port, EndPoint* ep) {
  if (ip_port.empty()) {
    return false;
  }
  size_t pos = ip_port.find(':');
  if (pos == std::string::npos) {
    return false;
  }
  std::string ip_str = ip_port.substr(0, pos);
  if (!StrToIP(ip_str, &ep->ip)) {
    return false;
  }

  std::string port_str = ip_port.substr(pos + 1);
  int port = std::atoi(port_str.c_str());
  if (port < 0 || port > 65535) {
    return false;
  }
  ep->port = port;
  return true;
}

bool EndPointToStr(const EndPoint& ep, std::string* ip_port) {
  char buf[INET_ADDRSTRLEN * 2] = {0};
  if (inet_ntop(AF_INET, &ep.ip, buf, INET_ADDRSTRLEN) == nullptr) {
    return false;
  }
  snprintf(buf + INET_ADDRSTRLEN, INET_ADDRSTRLEN, "%s:%d", buf, ep.port);
  ip_port->assign(buf + INET_ADDRSTRLEN);
  return true;
}

bool StrToIP(const char* ip_str, ip_t* ip) {
  return inet_pton(AF_INET, ip_str, ip) == 1;
}

bool StrToIP(const std::string& ip_str, ip_t* ip) {
  return StrToIP(ip_str.c_str(), ip);
}

bool IPToStr(ip_t ip, std::string* ip_str) {
  char buf[INET_ADDRSTRLEN] = {0};
  if (inet_ntop(AF_INET, &ip, buf, INET_ADDRSTRLEN) == nullptr) {
    return false;
  }
  ip_str->assign(buf);
  return true;
}

bool HostnameToIP(const char* hostname, ip_t* ip) {
  char buf[256];
  if (hostname == nullptr) {
    if (gethostname(buf, 256) == -1) {
      return false;
    }
    hostname = buf;
  }

  char aux_buf[1024];
  int error = 0;
  struct hostent ent;
  struct hostent* result = NULL;
  if (gethostbyname_r(hostname, &ent, aux_buf, sizeof(aux_buf),
                      &result, &error) != 0 || result == NULL) {
    return false;
  }
  // Only fetch the first address here
  bcopy(reinterpret_cast<char*>(result->h_addr), reinterpret_cast<char*>(ip),
        result->h_length);
  return true;
}

bool HostnameToIP(const std::string& hostname, ip_t* ip) {
  return HostnameToIP(hostname.c_str(), ip);
}

bool HostnameToEndPoint(const char* hostname, int port, EndPoint* ep) {
  ep->port = port;
  return HostnameToIP(hostname, &ep->ip);
}

bool HostnameToEndPoint(const std::string hostname, int port, EndPoint* ep) {
  return HostnameToEndPoint(hostname.c_str(), port, ep);
}

};  // namespace pink
