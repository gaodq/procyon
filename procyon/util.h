#pragma once

#include <pthread.h>
#include <string>

#include "procyon/endpoint.h"

namespace procyon {
namespace util {

uint64_t NowMicros();

int SetReuseAddr(int fd);
int SetNonblocking(int fd);
int SetFdCloseExec(int fd);
int SetNoDelay(int fd);

#if defined(__GLIBC__) && !defined(__APPLE__) && !defined(__ANDROID__)
# if __GLIBC_PREREQ(2, 12)
  // has pthread_setname_np(pthread_t, const char*) (2 params)
# define HAS_PTHREAD_SETNAME_NP 1
# endif
#endif

#ifdef HAS_PTHREAD_SETNAME_NP
inline bool SetThreadName(pthread_t id, const std::string& name) {
  //printf ("use pthread_setname_np(%s)\n", name.substr(0, 15).c_str());
  return 0 == pthread_setname_np(id, name.substr(0, 15).c_str());
}
#else
inline bool SetThreadName(pthread_t id, const std::string& name) {
  //printf ("no pthread_setname\n");
  return false;
}
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

bool AddrToEndPoint(const struct sockaddr_in* cliaddr, EndPoint* ep);
bool IPPortToEndPoint(const std::string& ip, int port, EndPoint* ep);
bool StrToEndPoint(const std::string& ip_port, EndPoint* ep);
bool EndPointToStr(const EndPoint& ep, std::string* ip_port);

bool StrToIP(const char* ip_str, ip_t* ip);
bool StrToIP(const std::string& ip_str, ip_t* ip);

bool IPToStr(ip_t ip, std::string* ip_str);

bool HostnameToIP(const char* hostname, ip_t* ip);
bool HostnameToIP(const std::string& hostname, ip_t* ip);

bool HostnameToEndPoint(const char* hostname, int port, EndPoint* ep);
bool HostnameToEndPoint(const std::string hostname, int port, EndPoint* ep);

}  // namespace util
}  // namespace procyon
