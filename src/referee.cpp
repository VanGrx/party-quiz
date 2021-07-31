#include "referee.h"

#include <iostream>

Referee::Referee() {}

Referee::Referee(int _playersNumber)
    : playersNumber{_playersNumber}, ready{true} {}

bool Referee::isReady() { return ready; }
