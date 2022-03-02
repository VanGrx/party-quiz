#include "game.h"
#include "gameconfig.h"

#include <thread>

#include "questionsgenerator.h"

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
  changeState(GAME_NULL);
}

int Game::addPlayer(std::string name) {

  int id = rand.getNextRandom();
  while (find_if(players.begin(), players.end(), [&id](const Player &p) {
           return id == p.id;
         }) != players.end())
    id = rand.getNextRandom();

  players.emplace_back(id, name);

  if (players.size() == playerNumber)
    changeState(GAME_READY);

  return id;
}

Question Game::giveQuestion() { return questions[currQuestion]; }

bool Game::nextRound() {
  currQuestion++;
  return true;
}

bool Game::gameFinished() { return state == GAME_FINISHED; }

bool Game::gameRunning() { return state == GAME_PLAYING; }

bool Game::gamePaused() { return state == GAME_PAUSED; }

bool Game::gameReady() { return state == GAME_READY; }

bool Game::allPlayersAnswered() {

  unsigned int currenQuestion = currQuestion;

  return std::all_of(players.begin(), players.end(),
                     [&currenQuestion](const Player &player) {
                       return player.questionsAnswered > currenQuestion;
                     });
}

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

  if (!gameRunning())
    return;

  auto player =
      find_if(players.begin(), players.end(),
              [&id](const auto &currPlayer) { return currPlayer.id == id; });

  player->gaveAnswer(currQuestion, answerGiven,
                     giveQuestion().correctAnswerIndex == answerGiven);
}

void Game::getQuestions() {

  QuestionsGenerator qg;

  questions = qg.collectQuestions();

  if (questions.empty())
    return;

  // We set that everything is OK and game is created
  changeState(GAME_CREATED);
  std::cout << "GAME CREATED" << std::endl;
}

void Game::startGame() {
  if (gamePlayingThread.joinable())
    gamePlayingThread.join();
  gamePlayingThread = std::thread(&Game::playGame, this);
}

void Game::playGame() {

  while (currQuestion < questions.size()) {

    changeState(GAME_PLAYING);

    unsigned timePassed = 0;

    while (timePassed < ROUND_TIME && !allPlayersAnswered()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      timePassed++;
    }
    changeState(GAME_PAUSED);
    std::this_thread::sleep_for(std::chrono::seconds(PAUSE_TIME));

    changeState(GAME_PREPARE);
    nextRound();
  }

  changeState(GAME_FINISHED);
}

void Game::changeState(const GameState state_) {
  gameMutex.lock();
  state = state_;
  gameMutex.unlock();
  callbackReceiver->stateChanged();
}

void Game::print() {

  for (auto it : questions)
    it.print();
}
