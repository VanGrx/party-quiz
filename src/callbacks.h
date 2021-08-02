#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "question.h"
#include <iostream>
#include <string>
#include <vector>

class CallbackListener {

public:
  // Scoreboard callbacks
  virtual void gameInitCallback(int id, int playerCount) = 0;
  virtual std::vector<std::pair<std::string, unsigned int>> getScores() = 0;
  virtual Question getQuestion() = 0;
  virtual std::string getGameStatusJSONString() = 0;
  virtual void startGame() = 0;

  // Player callbacks
  virtual bool playerEntered(int roomID, int id, std::string username) = 0;
  virtual void answerGiven(int id, int answerGiven) = 0;
};

#endif // CALLBACKS_H
