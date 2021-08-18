#ifndef LISTENER_H
#define LISTENER_H

#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "callbacks.h"
#include "game.h"
#include "http_session.h"
#include "utils.h"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Accepts incoming connections and launches the sessions
class Server : public std::enable_shared_from_this<Server>,
               public CallbackListener {

  int threads;
  std::shared_ptr<std::string const> doc_root_;
  net::io_context ioc_;
  tcp::acceptor acceptor_;

  Game game;

  std::mutex sessionsMutex;
  std::vector<std::shared_ptr<HttpSession>> httpSessions;
  std::vector<std::shared_ptr<WebSocketSession>> webSessions;

public:
  // Scoreboard callback
  virtual int gameInitCallback(int playerCount) override;
  virtual std::vector<std::pair<std::string, unsigned int>>
  getScores() override;
  virtual Question getQuestion() override;
  virtual void startGame() override;
  virtual std::string getGameStatusJSONString() override;
  virtual std::string getPlayerStatusJSONString(int id) override;

  virtual std::string getScoresJSONString() override;

  virtual bool gameExists(int id) override;

  // Player callbacks
  virtual int playerEntered(int roomID, std::string username) override;
  virtual void answerGiven(int id, int answerGiven) override;

  // WebSocket callbacks
  virtual bool
  webSocketConnected(std::shared_ptr<WebSocketSession> newSession) override;

  Server(int threads_, tcp::endpoint endpoint, std::string &doc_root);

  // Start accepting incoming connections
  void run();

  void start();

private:
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif // LISTENER_H
