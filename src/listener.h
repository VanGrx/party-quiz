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
#include "session.h"
#include "utils.h"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener>,
                 public CallbackListener {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  std::shared_ptr<std::string const> doc_root_;

  Game game;

  std::mutex sessionsMutex;
  std::vector<std::shared_ptr<Session>> sessions;

public:
  // Scoreboard callback
  virtual void gameInitCallback(int id, int playerCount) override;
  virtual std::vector<std::pair<std::string, unsigned int>>
  getScores() override;
  virtual Question getQuestion() override;
  virtual std::string getGameStatusJSONString() override;

  // Player callbacks
  virtual void playerEntered(int id, std::string username) override;
  virtual void answerGiven(int id, int answerGiven) override;

  Listener(net::io_context &ioc, tcp::endpoint endpoint,
           std::shared_ptr<std::string const> const &doc_root);

  // Start accepting incoming connections
  void run();

private:
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif // LISTENER_H
