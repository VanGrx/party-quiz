#ifndef GAME_H
#define GAME_H

#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "player.h"
#include "question.h"
#include "randomgenerator.h"

class Game {
public:
  Game();

  RandomGenerator rand;

  std::vector<Question> questions;
  std::vector<Player> players;

  int createGame(unsigned int _playerNumber);
  void clearGame();

  void getQuestions();
  bool gameReady();
  int addPlayer(std::string name);
  Question giveQuestion();
  bool nextRound();
  bool gameFinished();
  bool gameRunning();
  std::vector<std::pair<std::string, unsigned int>> getScores();
  void playerAnswered(int id, int answerGiven);

  Player getPlayer(int id);

  void print();

  void startGame();
  void playGame();

  std::mutex gameMutex;
  bool gameCreated = false;
  std::thread questionCacheThread;
  std::thread gamePlayingThread;

  int id = 0;

  bool gameStarted = false;
  unsigned int playerNumber;
  unsigned int currQuestion = 0;
};

#endif // GAME_H