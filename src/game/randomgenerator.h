#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

class RandomGenerator {
public:
  RandomGenerator() = delete;
  RandomGenerator(int _limit);

  int getNextRandom();

  int limit;
};

#endif // RANDOMGENERATOR_H
