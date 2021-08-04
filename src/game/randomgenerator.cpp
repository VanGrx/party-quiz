#include "randomgenerator.h"
#include <functional>
#include <memory>

RandomGenerator::RandomGenerator(int _limit) : limit{_limit} {
  srand(time(nullptr));
}

int RandomGenerator::getNextRandom() { return (rand() % limit) + 1; }
