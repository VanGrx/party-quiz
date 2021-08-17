#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "callbacks.h"
#include "pages.h"
#include "player.h"
#include "utils.h"

class Listener;

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

std::map<std::string, std::string> parse(const std::string &data);

// Handles an HTTP server connection
class Session : public std::enable_shared_from_this<Session> {
  // This is the C++11 equivalent of a generic lambda.
  // The function object is used to send an HTTP message.
  struct send_lambda {
    Session &self_;

    explicit send_lambda(Session &self) : self_(self) {}

    template <bool isRequest, class Body, class Fields>
    void operator()(http::message<isRequest, Body, Fields> &&msg) const {
      // The lifetime of the message has to extend
      // for the duration of the async operation so
      // we use a shared_ptr to manage it.
      auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(
          std::move(msg));

      // Store a type-erased version of the shared
      // pointer in the class to keep it alive.
      self_.res_ = sp;

      // Write the response
      http::async_write(self_.stream_, *sp,
                        beast::bind_front_handler(&Session::on_write,
                                                  self_.shared_from_this(),
                                                  sp->need_eof()));
    }
  };

  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  std::shared_ptr<std::string const> doc_root_;
  http::request<http::string_body> req_;
  std::shared_ptr<void> res_;
  send_lambda lambda_;

  std::shared_ptr<CallbackListener> callbackReceiver;

public:
  // Take ownership of the stream
  Session(tcp::socket &&socket,
          std::shared_ptr<std::string const> const &doc_root,
          std::shared_ptr<CallbackListener> _listener)
      : stream_(std::move(socket)), doc_root_(doc_root), lambda_(*this),
        callbackReceiver(_listener) {

    std::cout << "Creating session" << std::endl;
  }

  // Start the asynchronous operation
  void run();

  void do_read();

  void on_read(beast::error_code ec, std::size_t bytes_transferred);

  void on_write(bool close, beast::error_code ec,
                std::size_t bytes_transferred);

  void do_close();

  void generateID();

  std::string createPageRedirect(const int ID, const std::string &page);

  template <class Body, class Allocator>
  bool checkRequest(http::request<Body, http::basic_fields<Allocator>> &req);

  http::status parseBodyFromFile(std::string path,
                                 http::file_body::value_type &body);

  template <class Body, class Allocator, class Send>
  void handle_request(beast::string_view doc_root,
                      http::request<Body, http::basic_fields<Allocator>> &&req,
                      Send &&send);

  template <class Body, class Allocator, class Send>
  void
  handlePlayerRequest(beast::string_view doc_root,
                      http::request<Body, http::basic_fields<Allocator>> &&req,
                      Send &&send);

  template <class Body, class Allocator, class Send>
  void handleScoreboardRequest(
      beast::string_view doc_root,
      http::request<Body, http::basic_fields<Allocator>> &&req, Send &&send);

  template <class Body, class Allocator>
  http::response<http::string_body>
  createErrorResponse(http::request<Body, http::basic_fields<Allocator>> &&req,
                      http::status status, std::string what);

  template <class Body, class Allocator, class Send>
  void
  returnRequestedPage(const std::string &path,
                      http::request<Body, http::basic_fields<Allocator>> &&req,
                      Send &&send);

  template <class Body, class Allocator, class Send>
  void
  returnRequestedJSON(const std::string &jsonString,
                      http::request<Body, http::basic_fields<Allocator>> &&req,
                      Send &&send);
};

#endif // HTTP_SESSION_H
