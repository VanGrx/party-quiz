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
class websocket_session
    : public std::enable_shared_from_this<websocket_session> {
  websocket::stream<beast::tcp_stream> ws_;
  beast::flat_buffer buffer_;

public:
  // Take ownership of the socket
  explicit websocket_session(tcp::socket &&socket) : ws_(std::move(socket)) {}

  // Report a failure
  void fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
  }

  // Start the asynchronous accept operation
  template <class Body, class Allocator>
  void do_accept(http::request<Body, http::basic_fields<Allocator>> req) {
    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(
        websocket::stream_base::decorator([](websocket::response_type &res) {
          res.set(http::field::server,
                  std::string(BOOST_BEAST_VERSION_STRING) + " advanced-server");
        }));

    // Accept the websocket handshake
    ws_.async_accept(req,
                     beast::bind_front_handler(&websocket_session::on_accept,
                                               shared_from_this()));
  }

private:
  void on_accept(beast::error_code ec) {
    if (ec)
      return fail(ec, "accept");

    // Read a message
    do_read();
  }

  void do_read() {
    // Read a message into our buffer
    ws_.async_read(buffer_,
                   beast::bind_front_handler(&websocket_session::on_read,
                                             shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This indicates that the websocket_session was closed
    if (ec == websocket::error::closed)
      return;

    if (ec)
      fail(ec, "read");

    // TODO: Return what we must send and add functions to handle calls from
    // listener
    // Echo the message
    ws_.text(ws_.got_text());
    ws_.async_write(buffer_.data(),
                    beast::bind_front_handler(&websocket_session::on_write,
                                              shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
  }
};

#endif // WEBSOCKET_SESSION_H
