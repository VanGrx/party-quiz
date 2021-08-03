#include "game.h"
#include "utils.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <thread>

Game::Game() : rand(MAX_ID) {}

int Game::createGame(unsigned int _playerNumber) {
  clearGame();
  playerNumber = _playerNumber;
  id = rand.getNextRandom();
  if (questionCacheThread.joinable())
    questionCacheThread.join();
  questionCacheThread = std::thread(&Game::getQuestions, this);
  return id;
}

void Game::clearGame() {
  questions.clear();
  players.clear();
  currQuestion = 0;
  id = 0;
  gameCreated = false;
  gameStarted = false;
}

bool Game::gameReady() { return players.size() == playerNumber; }

int Game::addPlayer(std::string name) {

  int id = rand.getNextRandom();
  while (find_if(players.begin(), players.end(), [&id](const Player &p) {
           return id == p.id;
         }) != players.end())
    id = rand.getNextRandom();

  players.emplace_back(id, name);

  return id;
}

Question Game::giveQuestion() { return questions[currQuestion]; }

bool Game::nextRound() {
  currQuestion++;
  return true;
}

bool Game::gameFinished() { return currQuestion == questions.size(); }

bool Game::gameRunning() { return gameStarted && !gameFinished(); }

std::vector<std::pair<std::string, unsigned int>> Game::getScores() {
  std::vector<std::pair<std::string, unsigned int>> scores;

  std::for_each(players.begin(), players.end(), [&scores](auto &player) {
    scores.emplace_back(player.username, player.score);
  });

  sort(scores.begin(), scores.end(),
       [](const auto &p1, const auto &p2) { return p1.second > p2.second; });

  return scores;
}

Player Game::getPlayer(int id) {

  auto it = find_if(players.begin(), players.end(),
                    [&id](const Player &p) { return p.id == id; });
  if (it != players.end())
    return *it;
  return Player();
}

void Game::playerAnswered(int id, int answerGiven) {

  auto player =
      find_if(players.begin(), players.end(),
              [&id](const auto &currPlayer) { return currPlayer.id == id; });

  // If player already answered, ignore
  if (player->questionsAnswered >= currQuestion)
    return;

  player->gaveAnswer(giveQuestion().correctAnswerIndex == answerGiven);
}

void Game::getQuestions() {

  std::string response = curlCollect(
      "https://opentdb.com/api.php?amount=10&type=multiple&encode=base64");

  rapidjson::Document d;
  d.Parse(response.c_str());

  // Check if we parsed OK
  if (!d.IsObject())
    return;

  // Check if we have response_code
  if (!d.HasMember("response_code") || !d["response_code"].IsInt())
    return;

  // Check if response_code is OK
  if (d["response_code"].GetInt() != 0)
    return;

  const rapidjson::Value &a = d["results"];
  if (!a.IsArray())
    return;

  for (rapidjson::SizeType i = 0; i < a.Size(); i++) {
    auto obj = a[i].GetObject();
    auto question = Base64::decode(obj["question"].GetString());
    auto correctAnswer = Base64::decode(obj["correct_answer"].GetString());

    const rapidjson::Value &incorrect_answers = obj["incorrect_answers"];
    if (!incorrect_answers.IsArray())
      return;

    std::vector<std::string> answers;

    for (rapidjson::SizeType j = 0; j < incorrect_answers.Size(); j++) {
      answers.emplace_back(Base64::decode(incorrect_answers[j].GetString()));
    }

    questions.emplace_back(question, answers[0], answers[1], answers[2],
                           correctAnswer);
  }

  // We set that everything is OK and game is created
  gameMutex.lock();
  gameCreated = true;
  std::cout << "GAME CREATED" << std::endl;
  gameMutex.unlock();
}

void Game::startGame() {
  if (gamePlayingThread.joinable())
    gamePlayingThread.join();
  gamePlayingThread = std::thread(&Game::playGame, this);
}

void Game::playGame() {

  gameStarted = true;

  using namespace std::chrono_literals;

  while (!gameFinished()) {
    std::this_thread::sleep_for(20s);

    nextRound();
  }
}

void Game::print() {

  for (auto it : questions)
    it.print();
}
