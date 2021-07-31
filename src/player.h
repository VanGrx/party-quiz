#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player {
public:
  Player();

  Player(int _id, std::string _username)
      : id{_id}, username{_username}, score{0} {};

  void gaveAnswer(bool correct);

  unsigned int questionsAnswered = 0;
  int id;
  std::string username;
  unsigned int score;
};

#endif // PLAYER_H
