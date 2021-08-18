#ifndef WEBSOCKET_SESSION_H
#define WEBSOCKET_SESSION_H

#include <algorithm>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Echoes back all received WebSocket messages
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
  websocket::stream<beast::tcp_stream> ws_;
  beast::flat_buffer buffer_;

public:
  // Take ownership of the socket
  explicit WebSocketSession(tcp::socket &&socket) : ws_(std::move(socket)) {}

  // Report a failure
  void fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
  }

  // Start the asynchronous accept operation
  template <class Body, class Allocator>
  void do_accept(http::request<Body, http::basic_fields<Allocator>> req);

private:
  void on_accept(beast::error_code ec);

  void do_read();

  void on_read(beast::error_code ec, std::size_t bytes_transferred);

  void on_write(beast::error_code ec, std::size_t bytes_transferred);
};

#endif // WEBSOCKET_SESSION_H
