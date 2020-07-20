#ifndef GRAVITY_HPP
#define GRAVITY_HPP

#include "Common.hpp"
#include "Game.hpp"

namespace Gravity
{
    void step(bool haveGravity,
              Vec pos,
              Vec vel,
              Vec accel,
              Vec* pNewPos,
              Vec* pNewVel);

    void solve(int64_t minRadius,
               int64_t maxRadius,
               Vec startPos,
               Vec startVel,
               int64_t startTick,
               int64_t maxTicks,
               int64_t maxFuel,
               std::vector<Vec>* pAccels);
}

#endif
