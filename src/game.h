#ifndef GAME_H
#define GAME_H

#include <map>
#include <vector>

#include "player.h"
#include "question.h"

class Game {
public:
  Game();

  Game(unsigned int _playerNumber);

  std::vector<Question> questions;
  std::vector<Player> players;

  void getQuestions();
  bool gameReady();
  bool addPlayer(int id, std::string name);
  Question giveQuestion();
  bool nextRound();
  bool gameFinished();
  std::vector<std::pair<std::string, unsigned int>> getScores();
  void playerAnswered(int id, int answerGiven);

  void print();

  bool gameCreated = false;

private:
  bool gameStarted = false;
  unsigned int playerNumber;
  unsigned int currQuestion = 0;
};

#endif // GAME_H
