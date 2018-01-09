#include "src/util.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace pink {
namespace util {

int SetReuseAddr(int fd) {
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    return -1;
  }
  return 0;
}

int SetFdCloseExec(int fd) {
  // Set fd close exec
  return fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

int SetNonblocking(int fd) {
  int flags;
  if ((flags = fcntl(fd, F_GETFL, 0)) < 0) {
    close(fd);
    return -1;
  }
  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) {
    close(fd);
    return -1;
  }
  return flags;
}

}  // namespace util
}  // namespace pink
