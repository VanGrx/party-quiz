#ifndef REFEREE_H
#define REFEREE_H

#include "actor.h"

class Referee : public Actor {
public:
  Referee();

  Referee(int _playersNumber);

  bool isReady() override;

private:
  int playersNumber;
  bool ready = false;
};

#endif // REFEREE_H
