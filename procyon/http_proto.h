#pragma once

#include <memory>
#include <map>
#include <unordered_map>
#include <string>

#include "procyon/connection.h"
#include "procyon/iobuf.h"
#include "procyon/http_parser.h"

namespace procyon {

struct HTTPRequest {
  http_method method;
  std::string req_url;
  std::string path;
  std::string content_type;
  bool require_100continue;
  std::map<std::string, std::string> query_params;
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> postform_values;

  void Clear() {
    req_url.clear();
    path.clear();
    content_type.clear();
    require_100continue = false;
    query_params.clear();
    headers.clear();
    postform_values.clear();
  }
};

class HTTPMsgHandler {
 public:
  virtual ~HTTPMsgHandler() {}

  virtual void HandleNewRequest(Connection* conn, const HTTPRequest& msg) {}

  virtual void OnBody(Connection* conn, const char* data, size_t length) {}

  void WriteHeaders(Connection* conn, http_status status,
                    const std::unordered_map<std::string, std::string>& headers,
                    size_t content_length = 0);
  void WriteContent(Connection* conn, const char* data, size_t length);
  void WriteContent(Connection* conn, const std::string& msg) {
    return WriteContent(conn, msg.data(), msg.size());
  }

 private:
  void Write(Connection* conn, const char* data, size_t length) {
    conn->Write(data, length);
  }

  size_t remain_length_;
};

class HTTPConn : public Connection {
 public:
  explicit HTTPConn(HTTPMsgHandler* handler);

  virtual void OnDataAvailable(size_t size) override;

  virtual void GetReadBuffer(void** buffer, size_t* len) override;

 private:
  static int message_begin_cb(http_parser* parser);
  static int header_field_cb(http_parser* parser, const char *at, size_t length);
  static int header_value_cb(http_parser* parser, const char *at, size_t length);
  static int request_url_cb(http_parser* parser, const char *at, size_t length);
  static int response_status_cb(http_parser* parser, const char *at, size_t length) { return 0; }
  static int body_cb(http_parser* parser, const char *at, size_t length);
  static int headers_complete_cb(http_parser* parser);
  static int message_complete_cb(http_parser* parser);
  static int chunk_header_cb(http_parser* parser);
  static int chunk_complete_cb(http_parser* parser);

  HTTPMsgHandler* handler_;
  HTTPRequest http_req_;
  std::string tmp_header_k;

  int nparsed_;
  http_parser http_parser_;
  http_parser_settings parser_settings_;

  IOBuf buffer_;
};

}  // namespace procyon
