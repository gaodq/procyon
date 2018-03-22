#include "pink/dispatcher.h"

#include "pink/connection.h"
#include "pink/util.h"
#include "pink/xdebug.h"

namespace pink {

Dispatcher::Dispatcher(const ServerOptions& opts)
      : server_ip_(opts.listen_ip),
        server_port_(opts.port),
        accept_thread_(opts.accept_thread),
        worker_threads_(opts.worker_threads),
        error_cb_(opts.error_callback),
        close_cb_(opts.close_callback),
        ac_handler_(this),
        conn_factory_(opts.conn_factory) {
  connections_.reserve(10240);
}

bool Dispatcher::Bind() {
  if (server_socket_.Bind(server_ip_, server_port_) < 0) {
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
  ret = util::SetNoDelay(connfd);
  if (ret != 0) {
    log_warn("SetNoDelay failed");
  }

  auto t = worker_threads_->NextThread();

  auto conn = conn_factory_->NewConnection();
  EndPoint remote_side, local_side;
  util::IPPortToEndPoint(server_ip_, server_port_, &local_side);
  util::AddrToEndPoint(&cliaddr, &remote_side);
  conn->InitConn(connfd, t, this, &remote_side, &local_side);

  std::lock_guard<std::mutex> lock(conn_mu_);
  connections_.insert(std::make_pair(connfd, conn));
}

void Dispatcher::OnConnClosed(const Connection* conn) {
  // TODO run in loop
  log_info("Connection: %d closed", conn->fd());
  if (close_cb_) {
    close_cb_(conn);
  }
  std::lock_guard<std::mutex> lock(conn_mu_);
  connections_.erase(conn->fd());
}

void Dispatcher::OnConnError(const Connection* conn) {
  log_info("Connection: %d error", conn->fd());
  if (error_cb_) {
    error_cb_(conn);
  }
}

}  // namespace pink
