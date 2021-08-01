#include "listener.h"

Listener::Listener(net::io_context &ioc, tcp::endpoint endpoint,
                   std::shared_ptr<std::string const> const &doc_root)
    : ioc_(ioc), acceptor_(net::make_strand(ioc)), doc_root_(doc_root) {
  beast::error_code ec;

  // Open the acceptor
  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    fail(ec, "open");
    return;
  }

  // Allow address reuse
  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if (ec) {
    fail(ec, "set_option");
    return;
  }

  // Bind to the server address
  acceptor_.bind(endpoint, ec);
  if (ec) {
    fail(ec, "bind");
    return;
  }

  // Start listening for connections
  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    fail(ec, "listen");
    return;
  }
}

void Listener::run() { do_accept(); }

void Listener::do_accept() {
  // The new connection gets its own strand
  acceptor_.async_accept(
      net::make_strand(ioc_),
      beast::bind_front_handler(&Listener::on_accept, shared_from_this()));
}

void Listener::on_accept(beast::error_code ec, tcp::socket socket) {
  if (ec) {
    fail(ec, "accept");
  } else {
    // Create the session and run it
    sessions.emplace_back(std::make_shared<Session>(
        std::move(socket), doc_root_,
        std::static_pointer_cast<CallbackListener>(shared_from_this())));

    sessions.back()->run();
  }

  // Accept another connection
  do_accept();
}

//////////////////////////////////////////////////////////////////////////
/// Callbacks

void Listener::gameInitCallback(int playerCount) {
  game.createGame(playerCount);
}

std::vector<std::pair<std::string, unsigned int>> Listener::getScores() {

  return game.getScores();
}

Question Listener::getQuestion() { return game.giveQuestion(); }

// Player callbacks
void Listener::playerEntered(int id, std::string username) {
  game.addPlayer(id, username);
}

void Listener::answerGiven(int id, int answerGiven) {
  std::cout << "player entered" << id << " " << answerGiven;
}
