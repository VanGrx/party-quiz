#ifndef GAME_H
#define GAME_H

#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "callbacks.h"
#include "player.h"
#include "question.h"
#include "randomgenerator.h"

class Game {
public:
  Game();

  enum GameState {
    GAME_NULL = 0,
    GAME_CREATED,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_FINISHED,
    GAME_PREPARE,
    GAME_READY
  };

  int createGame(unsigned int _playerNumber,
                 std::shared_ptr<CallbackListener> _listener);
  void clearGame();

  void getQuestions();
  bool gameReady();
  int addPlayer(std::string name);
  Question giveQuestion();
  bool nextRound();
  bool gameFinished();
  bool gameRunning();
  bool gamePaused();
  bool allPlayersAnswered();
  std::vector<std::pair<std::string, unsigned int>> getScores();
  void playerAnswered(int id, int answerGiven);

  Player getPlayer(int id);

  void changeState(const GameState state_);
  void print();

  // Thread start and thread run functions
  void startGame();
  void playGame();

  std::thread questionCacheThread;
  std::thread gamePlayingThread;

  RandomGenerator rand;

  std::shared_ptr<CallbackListener> callbackReceiver;

  std::vector<Question> questions;
  std::vector<Player> players;

  std::mutex gameMutex;
  std::condition_variable cv;
  bool questionFinished = false;

  int id = 0;

  GameState state = GAME_NULL;

  unsigned int playerNumber = 0;
  unsigned int currQuestion = 0;
};

#endif // GAME_H
