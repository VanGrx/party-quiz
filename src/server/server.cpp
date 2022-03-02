#include "server.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

Server::Server(int threads_, std::string &doc_root)
    : threads{threads_}, doc_root_{std::make_shared<std::string>(doc_root)},
      ioc_{threads}, acceptor_(net::make_strand(ioc_)) {}

bool Server::init(tcp::endpoint endpoint) {

  beast::error_code ec;

  // Open the acceptor
  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    fail(ec, "open");
    return false;
  }

  // Allow address reuse
  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if (ec) {
    fail(ec, "set_option");
    return false;
  }

  // Bind to the server address
  acceptor_.bind(endpoint, ec);
  if (ec) {
    fail(ec, "bind");
    return false;
  }

  // Start listening for connections
  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    fail(ec, "listen");
    return false;
  }
  return true;
}

void Server::start() {
  // Capture SIGINT and SIGTERM to perform a clean shutdown
  net::signal_set signals(ioc_, SIGINT, SIGTERM);
  signals.async_wait([&](beast::error_code const &, int) {
    // Stop the `io_context`. This will cause `run()`
    // to return immediately, eventually destroying the
    // `io_context` and all of the sockets in it.
    ioc_.stop();
  });
  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads);
  for (auto i = threads; i > 0; --i)
    v.emplace_back([this] { ioc_.run(); });

  this->run();
  // (If we get here, it means we got a SIGINT or SIGTERM)

  // Block until all the threads exit
  for (auto &t : v)
    t.join();
}

void Server::run() { do_accept(); }

void Server::do_accept() {
  // The new connection gets its own strand
  acceptor_.async_accept(
      net::make_strand(ioc_),
      beast::bind_front_handler(&Server::on_accept, shared_from_this()));
}

void Server::on_accept(beast::error_code ec, tcp::socket socket) {
  if (ec) {
    fail(ec, "accept");
  } else {
    // Create the session and run it
    httpSessions.emplace_back(std::make_shared<HttpSession>(
        std::move(socket), doc_root_,
        std::static_pointer_cast<CallbackListener>(shared_from_this())));

    httpSessions.back()->run();
  }

  // Accept another connection
  do_accept();
}

//////////////////////////////////////////////////////////////////////////
/// Callbacks

int Server::gameInitCallback(int playerCount) {

  return game.createGame(playerCount, shared_from_this());
}

std::vector<std::pair<std::string, unsigned int>> Server::getScores() {

  return game.getScores();
}

Question Server::getQuestion() { return game.giveQuestion(); }

void Server::startGame() {
  if (game.gameReady())
    game.startGame();
}

bool Server::gameExists(int id) { return game.id == id; }

std::string Server::getGameStatusJSONString() {
  rapidjson::Document d;

  d.SetObject();

  d.AddMember("gameID", game.id, d.GetAllocator());

  d.AddMember("gameState", game.state, d.GetAllocator());
  d.AddMember("playerNumber", game.playerNumber, d.GetAllocator());
  d.AddMember("playersEntered", game.players.size(), d.GetAllocator());
  d.AddMember("totalQuestions", game.questions.size(), d.GetAllocator());
  d.AddMember("currQuestion", game.currQuestion, d.GetAllocator());

  if (game.gameRunning() || game.gamePaused()) {
    Question q = game.giveQuestion();

    rapidjson::Value jsonQ(q.question.c_str(), q.question.size(),
                           d.GetAllocator());
    d.AddMember("question", jsonQ, d.GetAllocator());

    rapidjson::Value jsonA(rapidjson::kArrayType);

    for (auto &it : q.answers) {
      rapidjson::Value ans(it.c_str(), it.size(), d.GetAllocator());

      jsonA.PushBack(ans, d.GetAllocator());
    }

    d.AddMember("answers", jsonA, d.GetAllocator());

    if (game.gamePaused()) {
      d.AddMember("correctAnswer", q.correctAnswerIndex, d.GetAllocator());
    }
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType,
                    rapidjson::ASCII<>>
      writer(buffer);

  d.Accept(writer);

  return buffer.GetString();
}

std::string Server::getPlayerStatusJSONString(int id) {
  rapidjson::Document d;

  d.SetObject();

  d.AddMember("gameState", game.state, d.GetAllocator());
  d.AddMember("playerNumber", game.playerNumber, d.GetAllocator());
  d.AddMember("playersEntered", game.players.size(), d.GetAllocator());
  d.AddMember("totalQuestions", game.questions.size(), d.GetAllocator());
  d.AddMember("currQuestion", game.currQuestion, d.GetAllocator());

  Player player = game.getPlayer(id);
  rapidjson::Value playerValue(rapidjson::kObjectType);

  playerValue.AddMember("id", player.id, d.GetAllocator());
  rapidjson::Value usr(player.username.c_str(), player.username.size(),
                       d.GetAllocator());
  playerValue.AddMember("username", usr, d.GetAllocator());
  playerValue.AddMember("score", player.score, d.GetAllocator());

  d.AddMember("player", playerValue, d.GetAllocator());

  if (game.gameRunning()) {
    Question q = game.giveQuestion();

    rapidjson::Value jsonQ(q.question.c_str(), q.question.size(),
                           d.GetAllocator());
    d.AddMember("question", jsonQ, d.GetAllocator());

    rapidjson::Value jsonA(rapidjson::kArrayType);

    for (auto &it : q.answers) {
      rapidjson::Value ans(it.c_str(), it.size(), d.GetAllocator());

      jsonA.PushBack(ans, d.GetAllocator());
    }

    d.AddMember("answers", jsonA, d.GetAllocator());
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType,
                    rapidjson::ASCII<>>
      writer(buffer);

  d.Accept(writer);

  return buffer.GetString();
}

std::string Server::getScoresJSONString() {
  rapidjson::Document d;

  d.SetArray();

  auto players = game.players;

  sort(players.begin(), players.end(),
       [](const Player &p1, const Player &p2) { return p1.score > p2.score; });

  for (auto &player : players) {
    rapidjson::Value res(rapidjson::kObjectType);

    res.AddMember("id", player.id, d.GetAllocator());

    rapidjson::Value username(player.username.c_str(), player.username.size(),
                              d.GetAllocator());
    res.AddMember("username", username, d.GetAllocator());
    res.AddMember("score", player.score, d.GetAllocator());

    d.PushBack(res, d.GetAllocator());
  }

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType,
                    rapidjson::ASCII<>>
      writer(buffer);

  d.Accept(writer);

  return buffer.GetString();
}

// Player callbacks
int Server::playerEntered(int roomID, std::string username) {

  if (game.id != roomID)
    return 0;
  return game.addPlayer(username);
}

void Server::answerGiven(int id, int answerGiven) {
  game.playerAnswered(id, answerGiven);
}

// WebSocket callbacks
bool Server::webSocketConnected(std::shared_ptr<WebSocketSession> newSession) {

  // Create the session and run it
  webSessions.push_back(newSession);

  return true;
}

// Game callbacks
void Server::stateChanged(int gameID) {

  for (auto ws : webSessions) {
    if (ws->getGameID() == gameID)
      ws->gameStateChanged();
  }
};
