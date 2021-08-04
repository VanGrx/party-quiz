#include "player.h"
#include "gameconfig.h"

Player::Player() {}

void Player::gaveAnswer(bool correct) {

  // TODO: Add beter logic for saving where player answered
  // Example: Some skips a question and answers twice for one and gets double
  // points
  questionsAnswered++;
  if (correct)
    score += POINTS_FOR_CORRECT_ANSWER;
}
