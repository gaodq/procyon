#pragma once

#include <memory>
#include <map>
#include <string>

#include "pink/connection.h"
#include "pink/iobuf.h"
#include "pink/http_parser.h"

namespace pink {

const size_t kDefaultBufferSize = 1024 * 1024;  // 1MB

const int kHTTPDelete = HTTP_DELETE;
const int kHTTPGet = HTTP_GET;
const int kHTTPHead = HTTP_HEAD;
const int kHTTPPost = HTTP_POST;
const int kHTTPPut = HTTP_PUT;
/* pathological */
const int kHTTPConnect = HTTP_CONNECT;
const int kHTTPOptions = HTTP_OPTIONS;
const int kHTTPTrace = HTTP_TRACE;
/* WebDAV */
const int kHTTPCopy = HTTP_COPY;
const int kHTTPLock = HTTP_LOCK;
const int kHTTPMkcol = HTTP_MKCOL;
const int kHTTPMove = HTTP_MOVE;
const int kHTTPPropfind = HTTP_PROPFIND;
const int kHTTPProppatch = HTTP_PROPPATCH;
const int kHTTPSearch = HTTP_SEARCH;
const int kHTTPUnlock = HTTP_UNLOCK;
const int kHTTPBind = HTTP_BIND;
const int kHTTPRebind = HTTP_REBIND;
const int kHTTPUnbind = HTTP_UNBIND;
const int kHTTPAcl = HTTP_ACL;
/* subversion */
const int kHTTPReport = HTTP_REPORT;
const int kHTTPMkactivity = HTTP_MKACTIVITY;
const int kHTTPCheckout = HTTP_CHECKOUT;
const int kHTTPMerge = HTTP_MERGE;
/* upnp */
const int kHTTPMsearch = HTTP_MSEARCH;
const int kHTTPNotify = HTTP_NOTIFY;
const int kHTTPSubscribe = HTTP_SUBSCRIBE;
const int kHTTPUnsunbscribe = HTTP_UNSUBSCRIBE;
/* RFC-5789 */
const int kHTTPPatch = HTTP_PATCH;
const int kHTTPPurge = HTTP_PURGE;
/* CalDAV */
const int kHTTPMkcalendar = HTTP_MKCALENDAR;
/* RFC-2068, section 19.6.1.2 */
const int kHTTPLink = HTTP_LINK;
const int kHTTPUnlink = HTTP_UNLINK;
/* icecast */
const int kHTTPSource = HTTP_SOURCE;

struct HTTPRequest {
  int method;
  std::string req_url;
  std::string path;
  std::string content_type;
  bool require_100continue;
  std::map<std::string, std::string> query_params;
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> postform_values;

  void Clear() {
    method = -1;
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

  void Write(Connection* conn, const std::string& msg) {
    conn->Write(msg.data(), msg.size());
  }
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

}  // namespace pink
