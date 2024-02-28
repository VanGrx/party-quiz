#include "randomgenerator.h"
#include <cstdlib>
#include <ctime>

RandomGenerator::RandomGenerator(int _limit) : limit{_limit} {
  srand(time(nullptr));
}

int RandomGenerator::getNextRandom() { return (rand() % limit) + 1; }
