#ifndef GAME_H
#define GAME_H

#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "player.h"
#include "question.h"

class Game {
public:
  Game();

  Game(unsigned int _playerNumber);

  std::vector<Question> questions;
  std::vector<Player> players;

  void createGame(unsigned int _playerNumber);
  void clearGame();

  void getQuestions();
  bool gameReady();
  bool addPlayer(int id, std::string name);
  Question giveQuestion();
  bool nextRound();
  bool gameFinished();
  std::vector<std::pair<std::string, unsigned int>> getScores();
  void playerAnswered(int id, int answerGiven);

  void print();

  std::mutex gameMutex;
  bool gameCreated = false;
  std::thread questionCacheThread;

  int id = 0;

  bool gameStarted = false;
  unsigned int playerNumber;
  unsigned int currQuestion = 0;
};

#endif // GAME_H
