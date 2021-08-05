#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>

class Player {
public:
  Player();

  Player(int _id, std::string _username)
      : id{_id}, username{_username}, score{0} {};

  void gaveAnswer(unsigned int questionIndex, unsigned int answer,
                  bool correct);

  unsigned int questionsAnswered = 0;
  std::vector<std::pair<unsigned int, unsigned int>> answersGiven;
  int id;
  std::string username;
  unsigned int score;
};

#endif // PLAYER_H
