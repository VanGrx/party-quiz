#include "listener.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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

int Listener::gameInitCallback(int playerCount) {

  return game.createGame(playerCount);
}

std::vector<std::pair<std::string, unsigned int>> Listener::getScores() {

  return game.getScores();
}

Question Listener::getQuestion() { return game.giveQuestion(); }

void Listener::startGame() {
  if (game.gameReady())
    game.startGame();
}

bool Listener::gameExists(int id) { return game.id == id; }

std::string Listener::getGameStatusJSONString() {
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

std::string Listener::getPlayerStatusJSONString(int id) {
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

std::string Listener::getScoresJSONString() {
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
int Listener::playerEntered(int roomID, std::string username) {

  if (game.id != roomID)
    return 0;
  return game.addPlayer(username);
}

void Listener::answerGiven(int id, int answerGiven) {
  game.playerAnswered(id, answerGiven);
}
