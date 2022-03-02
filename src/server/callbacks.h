#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "question.h"
#include "websocket_session.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class CallbackListener {

public:
  // Scoreboard callbacks
  virtual int gameInitCallback(int playerCount) = 0;
  virtual std::vector<std::pair<std::string, unsigned int>> getScores() = 0;
  virtual Question getQuestion() = 0;
  virtual std::string getGameStatusJSONString() = 0;
  virtual std::string getPlayerStatusJSONString(int id) = 0;
  virtual void startGame() = 0;
  virtual std::string getScoresJSONString() = 0;
  virtual bool gameExists(int id) = 0;

  // Player callbacks
  virtual int playerEntered(int roomID, std::string username) = 0;
  virtual void answerGiven(int id, int answerGiven) = 0;

  // WebSocket callbacks
  virtual bool
  webSocketConnected(std::shared_ptr<WebSocketSession> newSession) = 0;

  // Game callbacks
  virtual void stateChanged(int gameID) = 0;
};

#endif // CALLBACKS_H
