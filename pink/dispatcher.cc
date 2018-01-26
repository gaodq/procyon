#include "pink/dispatcher.h"

#include "pink/util.h"
#include "pink/xdebug.h"

namespace pink {

Dispatcher::Dispatcher(const ServerOptions& opts)
      : ip_(opts.listen_ip),
        port_(opts.port),
        accept_thread_(opts.accept_thread),
        worker_threads_(opts.worker_threads),
        ac_handler_(this),
        conn_factory_(opts.conn_factory) {
  connections_.reserve(10240);
}

bool Dispatcher::Bind() {
  if (server_socket_.Bind(ip_, port_) < 0) {
    return false;
  }
  if (server_socket_.Listen()) {
    return false;
  }
  if (!ac_handler_.RegisterHandler(accept_thread_->event_loop(),
                                   server_socket_.fd())) {
    return false;
  }

  return true;
}

void Dispatcher::OnNewConnection() {
  log_info("New connection");
  struct sockaddr_in cliaddr;
  socklen_t clilen = sizeof(struct sockaddr);
  int connfd = accept(server_socket_.fd(), (struct sockaddr *) &cliaddr, &clilen);
  if (connfd == -1) {
    log_warn("accept error, errno numberis %d, error reason %s",
             errno, strerror(errno));
    return;
  }
  int ret;
  ret = util::SetFdCloseExec(connfd);
  if (ret != 0) {
    log_warn("SetFdCloseExec failed");
  }
  ret = util::SetNonblocking(connfd);
  if (ret != 0) {
    log_warn("SetNonblocking failed");
  }

  auto t = worker_threads_->NextThread();

  auto conn = conn_factory_->NewConnection();
  if (!conn->InitConnection(connfd, t, this)) {
    log_err("init connection failed, connfd: %d", connfd);
  }

  connections_.insert(std::make_pair(connfd, conn));
}

void Dispatcher::OnConnClosed(Connection* conn) {
  // TODO run in loop
  connections_.erase(conn->fd());
}

}  // namespace pink
