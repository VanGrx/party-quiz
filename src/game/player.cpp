#include "player.h"
#include "gameconfig.h"

Player::Player() {}

void Player::gaveAnswer(unsigned int questionIndex, unsigned int answer,
                        bool correct) {

  if (questionIndex < questionsAnswered)
    return;

  auto it = find_if(answersGiven.begin(), answersGiven.end(),
                    [&questionIndex](const auto &answer) {
                      return answer.first == questionIndex;
                    });

  if (it != answersGiven.end())
    return;
  answersGiven.emplace_back(questionIndex, answer);
  questionsAnswered = questionIndex + 1;
  if (correct)
    score += POINTS_FOR_CORRECT_ANSWER;
}
