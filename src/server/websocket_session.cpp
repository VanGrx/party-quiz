#include "websocket_session.h"
#include "callbacks.h"
#include <cstdlib>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>

void WebSocketSession::on_accept(beast::error_code ec) {
  if (ec)
    return fail(ec, "accept");

  // Read a message
  do_read();
}

void WebSocketSession::handle_request() {

  ws_.text(ws_.got_text());
  std::string got_text = beast::buffers_to_string(buffer_.data());

  rapidjson::Document document;

  if (document.Parse<0>(got_text.c_str()).HasParseError()) {
    return do_write("Bad json file given");
  }

  if (!document.HasMember("type") || !document["type"].IsString())
    return do_write("Bad json file given");

  if (document["type"] == "player")
    handlePlayerRequest(document);
  else if (document["type"] == "scoreboard")
    handleScoreboardRequest(document);
  else
    return do_write("Bad json file given");
}

void WebSocketSession::handlePlayerRequest(
    const rapidjson::Document &document) {

  if (!document.HasMember("method") || !document["method"].IsString())
    return do_write("Bad json file given");

  if (document["method"] == "gameInit") {
    if (!document.HasMember("roomNumber") || !document["roomNumber"].IsInt())
      return do_write("Bad json file given");
    if (!document.HasMember("username") || !document["username"].IsString())
      return do_write("Bad json file given");

    gameID = document["roomNumber"].GetInt();
    std::string username = document["username"].GetString();

    playerID = callbackReceiver->playerEntered(gameID, username);

    if (playerID == 0) {
      do_write("Invalid ID");
    }

    std::string message = callbackReceiver->getPlayerStatusJSONString(playerID);

    do_write(message);
  } else if (document["method"] == "startGame") {
    callbackReceiver->startGame(playerID);

  } else if (document["method"] == "giveAnswer") {
    if (!document.HasMember("answer") || !document["answer"].IsNumber())
      return do_write("Bad json file given");
    int answer = document["answer"].GetInt();
    callbackReceiver->answerGiven(playerID, answer);
  }
}
void WebSocketSession::handleScoreboardRequest(
    const rapidjson::Document &document) {

  if (!document.HasMember("method") || !document["method"].IsString())
    return do_write("Bad json file given");

  if (document["method"] == "gameInit") {

    gameID = callbackReceiver->gameInitCallback();

    std::string message = callbackReceiver->getGameStatusJSONString();

    do_write(message);
  } else if (document["method"] == "gameStart") {
    //callbackReceiver->startGame();
  } else if (document["method"] == "getScores") {
    std::string message = callbackReceiver->getScoresJSONString();
    do_write(message);
  }
}

void WebSocketSession::async_write() {

  std::string message;

  writeMutex.lock();
  if (!writeQueue.empty())
    message = writeQueue.at(0);
  else {
    writeMutex.unlock();
    return;
  }
  writeMutex.unlock();

  beast::flat_buffer write_buffer;
  write_buffer.commit(buffer_copy(write_buffer.prepare(message.size()),
                                  boost::asio::buffer(message)));

  ws_.async_write(write_buffer.data(),
                  beast::bind_front_handler(&WebSocketSession::on_write,
                                            shared_from_this()));
}

void WebSocketSession::do_write(const std::string &message) {

  writeMutex.lock();
  writeQueue.push_back(message);
  // If we already have to write, leave it
  if (writeQueue.size() > 1) {
    writeMutex.unlock();
    return;
  }
  writeMutex.unlock();

  // TODO: Should probably be some logic if too many messages are left, right?

  async_write();
}

void WebSocketSession::on_write(beast::error_code ec,
                                std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return fail(ec, "write");

  writeMutex.lock();
  writeQueue.erase(writeQueue.begin());
  if (!writeQueue.empty()) {
    writeMutex.unlock();
    async_write();
    return;
  }
  writeMutex.unlock();
}

void WebSocketSession::do_read() {

  // Read a message into our buffer
  ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketSession::on_read,
                                                    shared_from_this()));
}

void WebSocketSession::on_read(beast::error_code ec,
                               std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  // This indicates that the WebSocketSession was closed
  if (ec == websocket::error::closed) {
    fail(ec, "close");
    return;
  }

  if (ec) {
    fail(ec, "read");
    return;
  }

  // Send the response
  handle_request();

  // Clear the buffer
  buffer_.consume(buffer_.size());

  // Do another read
  do_read();
}

// Start the asynchronous accept operation
template <class Body, class Allocator>
void WebSocketSession::do_accept(
    http::request<Body, http::basic_fields<Allocator>> req) {
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
  ws_.async_accept(req, beast::bind_front_handler(&WebSocketSession::on_accept,
                                                  shared_from_this()));
}

void WebSocketSession::fail(beast::error_code ec, char const *what) {

  std::cerr << "WebSocket " << what << ": " << ec.message() << "\n";
}

void WebSocketSession::gameStateChanged() {

  if (playerID == 0) {
    do_write(callbackReceiver->getGameStatusJSONString());
    do_write(callbackReceiver->getScoresJSONString());
  } else {
    do_write(callbackReceiver->getPlayerStatusJSONString(playerID));
  }
}

template void WebSocketSession::do_accept<http::string_body>(
    http::request<http::string_body>);
