#include "player.h"
#include "utils.h"

Player::Player() {}

void Player::gaveAnswer(bool correct) {
  questionsAnswered++;
  if (correct)
    score += POINTS_FOR_CORRECT_ANSWER;
}
