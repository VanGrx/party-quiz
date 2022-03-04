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
#include <rapidjson/document.h>
#include <string>
#include <thread>
#include <vector>

class CallbackListener;

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
  explicit WebSocketSession(tcp::socket &&socket,
                            std::shared_ptr<CallbackListener> _listener)
      : ws_(std::move(socket)), callbackReceiver(_listener) {
    std::cout << "Creating WebSocket session" << std::endl;
  }

  // Report a failure
  void fail(beast::error_code ec, char const *what);

  // Start the asynchronous accept operation
  template <class Body, class Allocator>
  void do_accept(http::request<Body, http::basic_fields<Allocator>> req);

  int getGameID() { return gameID; };

  void gameStateChanged();

private:
  std::shared_ptr<CallbackListener> callbackReceiver;
  int gameID;
  int playerID = 0;

  std::mutex writeMutex;
  std::vector<std::string> writeQueue;

  void on_accept(beast::error_code ec);

  void do_read();

  void on_read(beast::error_code ec, std::size_t bytes_transferred);

  void do_write(const std::string &message);
  void async_write();

  void on_write(beast::error_code ec, std::size_t bytes_transferred);

  void handle_request();

  void handlePlayerRequest(const rapidjson::Document &document);
  void handleScoreboardRequest(const rapidjson::Document &document);
};

#endif // WEBSOCKET_SESSION_H
