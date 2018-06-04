#include "procyon/http_proto.h"
#include "procyon/xdebug.h"
#include <algorithm>
#include <sstream>

namespace procyon {

int HTTPConn::message_begin_cb(http_parser* parser) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  if (conn->state() != Connection::kConnected) {
    return -1;
  }
  log_info("message_begin, content-length: %lu", parser->content_length);
  conn->http_req_.Clear();
  conn->http_req_.method = static_cast<http_method>(parser->method);

  return 0;
}

int HTTPConn::header_field_cb(http_parser* parser,
                              const char *at, size_t length) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  if (conn->state() != Connection::kConnected) {
    return -1;
  }
  conn->tmp_header_k.append(at, length);

  return 0;
}

int HTTPConn::header_value_cb(http_parser* parser,
                              const char *at, size_t length) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  if (conn->state() != Connection::kConnected) {
    return -1;
  }
  std::string& k = conn->tmp_header_k;
  std::string v(at, length);
  std::transform(k.begin(), k.end(), k.begin(), ::tolower);
  if (k == "host") {
    // Format path
    const std::string& path = conn->http_req_.path;
    size_t pos = path.find(v);
    size_t path_pos = 7 + v.size();  // http://www.xxx.xxx/path_/to
    if (pos != std::string::npos && path.size() > path_pos) {
      conn->http_req_.path.assign(path.substr(path_pos));
    }
  }
  conn->http_req_.headers.insert(std::make_pair(k, v));
  conn->tmp_header_k.clear();

  return 0;
}

int HTTPConn::request_url_cb(http_parser* parser,
                             const char *at, size_t length) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  if (conn->state() != Connection::kConnected) {
    return -1;
  }
  conn->http_req_.req_url.assign(at, length);

  size_t i = 0;
  for (; i < length; i++) {
    char ch = *(at + i);
    if (ch == '?') {
      break;
    }
  }
  conn->http_req_.path.assign(at, i);

  if (i < length) {
    std::string param_str(at + i + 1, length - i - 1);
    std::vector<std::string> elems;
    std::stringstream ss(param_str);
    std::string item;
    while (std::getline(ss, item, '&')) {
      if (!item.empty())
        elems.push_back(item);
    }
    for (const auto& e : elems) {
      size_t pos = e.find('=');
      std::string k, v;
      if (pos == std::string::npos) {
        k = e;
      } else {
        k = e.substr(0, pos);
        v = e.substr(pos + 1);
      }
      conn->http_req_.query_params.insert(std::make_pair(k, v));
    }
  }

  return 0;
}

int HTTPConn::body_cb(http_parser* parser,
                      const char *at, size_t length) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  if (conn->state() != Connection::kConnected) {
    return -1;
  }
  log_info("body_cb, length: %lu", length);
  conn->handler_->OnBody(conn->getptr(), at, length);

  return 0;
}

int HTTPConn::headers_complete_cb(http_parser* parser) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  if (conn->state() != Connection::kConnected) {
    return -1;
  }
  conn->http_req_.content_length = parser->content_length;
  conn->handler_->OnNewRequest(conn->getptr(), conn->http_req_);

  return 0;
}

int HTTPConn::message_complete_cb(http_parser* parser) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  if (conn->state() != Connection::kConnected) {
    return -1;
  }
  conn->handler_->OnComplete(conn->getptr());

  log_info("message_complete");
  http_parser_init(parser, HTTP_REQUEST);

  return 0;
}

int HTTPConn::chunk_header_cb(http_parser* parser) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  log_info("chunk_header_cb, content-length: %lu", parser->content_length);
  conn->handler_->OnChunkBegin(conn->getptr(), parser->content_length);

  return 0;
}

int HTTPConn::chunk_complete_cb(http_parser* parser) {
  HTTPConn* conn = reinterpret_cast<HTTPConn*>(parser->data);
  log_info("chunk_complete_cb");
  conn->handler_->OnChunkComplete(conn->getptr());

  return 0;
}

std::future<bool> HTTPMsgHandler::WriteHeaders(
    ConnectionPtr conn, http_status status,
    const std::unordered_map<std::string, std::string>& headers,
    size_t content_length) {
  char buf[256];
  std::string headers_msg;
  int len = snprintf(buf, 256, "HTTP/1.1 %d %s\r\nContent-Length: %lu\r\n",
                     http_status_code(status), http_status_name(status),
                     content_length);
  headers_msg.append(buf, len);
  for (const auto& item : headers) {
    headers_msg.append(item.first);
    headers_msg.append(": ");
    headers_msg.append(item.second);
    headers_msg.append("\r\n");
  }
  headers_msg.append("\r\n");

  return Write(conn, headers_msg.data(), headers_msg.size());
}

std::future<bool> HTTPMsgHandler::WriteContent(
    ConnectionPtr conn,
    const char* data, size_t length) {
  return Write(conn, data, length);
}

HTTPConn::HTTPConn(HTTPMsgHandler* handler)
    : handler_(handler) {
  parser_settings_.on_message_begin = message_begin_cb;
  parser_settings_.on_header_field = header_field_cb;
  parser_settings_.on_header_value = header_value_cb;
  parser_settings_.on_url = request_url_cb;
  parser_settings_.on_status = response_status_cb;
  parser_settings_.on_body = body_cb;
  parser_settings_.on_headers_complete = headers_complete_cb;
  parser_settings_.on_message_complete = message_complete_cb;
  parser_settings_.on_chunk_header = chunk_header_cb;
  parser_settings_.on_chunk_complete = chunk_complete_cb;

  http_parser_init(&http_parser_, HTTP_REQUEST);
  http_parser_.data = this;
}

void HTTPConn::OnDataAvailable(size_t size) {
  const char* data = buffer_.tail();
  buffer_.PostAllocate(size);
  int nparsed = http_parser_execute(&http_parser_, &parser_settings_,
                                    data, size);
  buffer_.TrimStart(nparsed);
}

void HTTPConn::GetReadBuffer(void** buffer, size_t* len) {
  auto mem = buffer_.PreAllocate();
  *buffer = mem.first;
  *len = mem.second;
}

}  // namespace procyon
