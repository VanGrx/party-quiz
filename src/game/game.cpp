#include "game.h"
#include "gameconfig.h"

#include <cstdint>
#include <thread>

#include "questionsgenerator.h"

Game::Game() : rand(MAX_ID) {}

int Game::createGame(std::shared_ptr<CallbackListener> _listener) {
  clearGame();
  id = rand.getNextRandom();
  callbackReceiver = _listener;
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

  if (players.empty()) {
    mainPlayerID = id;
  }

  players.emplace_back(id, name);

  playerAdded();

  // TODO: Add this when main player says to start
  //  changeState(GAME_READY);

  return id;
}

void Game::playerAdded() { callbackReceiver->stateChanged(id); }

Question Game::giveQuestion() { return questions[currQuestion]; }

uint8_t Game::getRoundTime() { return ROUND_TIME; }

uint8_t Game::getPauseTime() { return PAUSE_TIME; }

bool Game::nextRound() {
  currQuestion++;
  return true;
}

bool Game::gameFinished() { return state == GAME_FINISHED; }

bool Game::gameRunning() { return state == GAME_PLAYING; }

bool Game::gamePaused() { return state == GAME_PAUSED; }

bool Game::gameReady() { return state == GAME_READY; }

bool Game::gameCreated() { return state == GAME_CREATED; }

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

  if (allPlayersAnswered()) {
    std::unique_lock<std::mutex> lk(gameMutex);
    questionFinished = true;
    lk.unlock();
    cv.notify_all();
  }
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

    {
      std::unique_lock<std::mutex> lk(gameMutex);
      questionFinished = false;
      lk.unlock();
    }

    changeState(GAME_PLAYING);

    std::unique_lock<std::mutex> lk(gameMutex);
    cv.wait_for(lk, std::chrono::seconds(ROUND_TIME),
                [this] { return questionFinished; });
    lk.unlock();

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
  if (callbackReceiver && state != GAME_NULL)
    callbackReceiver->stateChanged(id);
}

void Game::print() {

  for (auto it : questions)
    it.print();
}
