#include <unistd.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

#include "procyon/server.h"
#include "procyon/options.h"
#include "procyon/connection.h"
#include "procyon/redis_proto.h"
#include "procyon/io_thread.h"
#include "procyon/bg_thread.h"

#include "nemo.h"

class CustomHandler : public procyon::RedisMsgHandler {
 public:
  explicit CustomHandler(procyon::BGThread* wts, nemo::Nemo* db)
      : worker_threads_(wts),
        db_(db) {
  }

  struct RedisArgs {
    procyon::Connection* conn;
    std::string command;
    std::vector<std::string> args;
    nemo::Nemo* db;
  };

  static void HandleDBRequest(void* arg) {
    RedisArgs* redis_args = reinterpret_cast<RedisArgs*>(arg);
    const std::string& command = redis_args->command;
    const std::vector<std::string> args = redis_args->args;

    if (command == "set" ||
        command == "SET") {
      const std::string& key = args[0];
      const std::string& value = args[1];
      if (key < "key:000000000010") {
        usleep(5000);  // 1ms
      }
      nemo::Status s = redis_args->db->Set(key, value);
      if (!s.ok()) {
        std::cout << s.ToString() << std::endl;
      }
    } else if (command == "get" ||
               command == "GET") {
      std::string val;
      nemo::Status s = redis_args->db->Get(args[0], &val);
      char buf[1024];
      int len = snprintf(buf, 1024, "$%lu\r\n%s\r\n", val.size(), val.c_str());
      redis_args->conn->Write(buf, len);
      delete redis_args;
      return;
    } else {
      std::cout << command << std::endl;
    }

    redis_args->conn->Write("+OK\r\n", 5);
    delete redis_args;
  }

  virtual void HandleRedisMsg(procyon::Connection* conn,
                              const std::string& command,
                              const std::vector<std::string>& args) override {

    RedisArgs* redis_args = new RedisArgs;
    redis_args->command.assign(std::move(command));
    redis_args->args = std::move(args);
    redis_args->conn = conn;
    redis_args->db = db_;

    worker_threads_->Schedule(HandleDBRequest, redis_args);
    // HandleDBRequest(redis_args);
  }

 private:
  nemo::Nemo* const db_;
  procyon::BGThread* const worker_threads_;
};

class MyConnFactory : public procyon::ConnectionFactory {
 public:
  MyConnFactory(procyon::BGThread* wts, nemo::Nemo* db)
      : worker_threads_(wts),
        db_(db) {
  }
  std::shared_ptr<procyon::Connection> NewConnection() override {
    return std::make_shared<procyon::LineBasedConn>(
      new CustomHandler(worker_threads_, db_));
  }
 private:
  nemo::Nemo* const db_;
  procyon::BGThread* const worker_threads_;
};

int main() {
  std::string db_path("./redis_db");
  nemo::Options nemo_opts;
  nemo::Nemo* db = new nemo::Nemo(db_path, nemo_opts);

  procyon::BGThread worker_threads(10);
  worker_threads.Start();

  procyon::ServerOptions server_opts;
  server_opts.port = 6379;
  server_opts.conn_factory =
    std::make_shared<MyConnFactory>(&worker_threads, db);
  procyon::Server server(server_opts);
  bool res = server.Start();
  if (!res) {
    std::cout << "Error" << std::endl;
  }

  while (true) {
    sleep(10);
  }

  return 0;
}
