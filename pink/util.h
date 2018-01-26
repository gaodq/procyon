#ifndef PINK_UTIL_H_
#define PINK_UTIL_H_

#include <pthread.h>
#include <string>

namespace pink {
namespace util {

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

}  // namespace util
}  // namespace pink

#endif  //  PINK_UTIL_H_
