#include "procyon/dispatcher.h"

#include "procyon/connection.h"
#include "procyon/util.h"
#include "procyon/xdebug.h"

namespace procyon {

Dispatcher::Dispatcher(const ServerOptions& opts)
      : opts_(opts),
        ac_handler_(this) {
  connections_.reserve(10240);
}

bool Dispatcher::Bind() {
  if (server_socket_.Bind(opts_.listen_ip, opts_.port) < 0) {
    return false;
  }
  if (server_socket_.Listen()) {
    return false;
  }
  if (!ac_handler_.RegisterHandler(opts_.accept_thread->event_loop(),
                                   server_socket_.fd())) {
    return false;
  }

  opts_.accept_thread->event_loop()->RunInLoop([this](){
    std::lock_guard<std::mutex> l(conn_mu_);
    for (auto& item : connections_) {
      auto& conn = item.second;
      if (conn->IdleSeconds() > opts_.connection_idle_timeout_s) {
        conn->Close();
      }
    }
  }, false);

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

  auto t = opts_.worker_threads->NextThread();

  auto conn = opts_.conn_factory->NewConnection();
  EndPoint remote_side, local_side;
  util::IPPortToEndPoint(opts_.listen_ip, opts_.port, &local_side);
  util::AddrToEndPoint(&cliaddr, &remote_side);
  conn->InitConn(connfd, t, this, &remote_side, &local_side);

  {
  std::lock_guard<std::mutex> lock(conn_mu_);
  connections_[connfd] = conn;
  }

  if (opts_.newconn_callback) {
    opts_.newconn_callback(conn);
  }
}

void Dispatcher::OnConnClosed(int conn_fd) {
  std::lock_guard<std::mutex> lock(conn_mu_);
  if (connections_.count(conn_fd)) {
    ConnectionPtr conn = connections_[conn_fd];
    if (opts_.close_callback) {
      opts_.close_callback(conn);
    }
    connections_.erase(conn_fd);
  }
}

void Dispatcher::OnConnError(int conn_fd) {
  log_info("Connection: %d error", conn_fd);
  std::lock_guard<std::mutex> lock(conn_mu_);
  if (connections_.count(conn_fd)) {
    ConnectionPtr conn = connections_[conn_fd];
    if (opts_.error_callback) {
      opts_.error_callback(conn);
    }
    connections_.erase(conn_fd);
  }
}

}  // namespace procyon
