#include "game.h"
#include "utils.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

Game::Game() {}

Game::Game(unsigned int _playerNumber) : playerNumber{_playerNumber} {
  // TODO: Collect questions in separate thread
  getQuestions();
  gameCreated = true;
}

bool Game::gameReady() { return players.size() == playerNumber; }

bool Game::addPlayer(int id, std::string name) {
  players.emplace_back(id, name);

  return true;
}

Question Game::giveQuestion() { return questions[currQuestion]; }

bool Game::nextRound() {
  currQuestion++;
  return true;
}

bool Game::gameFinished() { return currQuestion == questions.size(); }

std::vector<std::pair<std::string, unsigned int>> Game::getScores() {
  std::vector<std::pair<std::string, unsigned int>> scores;

  std::for_each(players.begin(), players.end(), [&scores](auto &player) {
    scores.emplace_back(player.username, player.score);
  });

  sort(scores.begin(), scores.end(),
       [](const auto &p1, const auto &p2) { return p1.second > p2.second; });

  return scores;
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
}

void Game::print() {

  for (auto it : questions)
    it.print();
}
