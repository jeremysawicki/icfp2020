#include "Gravity.hpp"

using std::vector;

namespace Gravity
{
    void step(bool haveGravity,
              Vec pos,
              Vec vel,
              Vec accel,
              Vec* pNewPos,
              Vec* pNewVel)
    {
        Vec newVel = vel;
        newVel.m_x -= accel.m_x;
        newVel.m_y -= accel.m_y;

        if (haveGravity)
        {
            if (pos.m_x > 0 &&
                pos.m_y <= pos.m_x &&
                pos.m_y >= -pos.m_x)
            {
                newVel.m_x--;
            }

            if (pos.m_x < 0 &&
                pos.m_y >= pos.m_x &&
                pos.m_y <= -pos.m_x)
            {
                newVel.m_x++;
            }

            if (pos.m_y > 0 &&
                pos.m_x <= pos.m_y &&
                pos.m_x >= -pos.m_y)
            {
                newVel.m_y--;
            }

            if (pos.m_y < 0 &&
                pos.m_x >= pos.m_y &&
                pos.m_x <= -pos.m_y)
            {
                newVel.m_y++;
            }
        }

        Vec newPos = pos;
        newPos.m_x += newVel.m_x;
        newPos.m_y += newVel.m_y;

        if (pNewPos) *pNewPos = newPos;
        if (pNewVel) *pNewVel = newVel;
    }

    void solve(int64_t minRadius,
               int64_t maxRadius,
               Vec startPos,
               Vec startVel,
               int64_t maxTicks,
               int64_t maxFuel,
               vector<Vec>* pAccels)
    {
        vector<Vec> accels(maxTicks);

        auto isBad = [&](const Vec& pos) -> bool
        {
            return
            (pos.m_x >= -minRadius &&
             pos.m_x <= minRadius &&
             pos.m_y >= -minRadius &&
             pos.m_y <= minRadius) ||
            pos.m_x < -maxRadius ||
            pos.m_x > maxRadius ||
            pos.m_y < -maxRadius ||
            pos.m_y > maxRadius;
        };

        if (isBad(startPos))
        {
            printf("Start position is bad!\n");
            *pAccels = std::move(accels);
            return;
        }

        auto check = [&]() -> int64_t
        {
            Vec pos = startPos;
            Vec vel = startVel;
            for (int64_t tick = 0; tick < maxTicks; tick++)
            {
                step(true, pos, vel, accels[tick], &pos, &vel);
                if (isBad(pos))
                {
                    return tick;
                }
            }
            return maxTicks;
        };

        Vec dirs[8] = {
            { -1, -1 },
            { -1,  1 },
            {  1, -1 },
            {  1,  1 },
            { -1,  0 },
            {  0, -1 },
            {  1,  0 },
            {  0,  1 }
        };

        int64_t neededFuel = 0;
        int64_t curTicks = 0;

        while (true)
        {
            curTicks = check();
            printf("Ticks: %" PRIi64 " of %" PRIi64 "  Fuel: %" PRIi64 " of %" PRIi64 "\n", curTicks, maxTicks, neededFuel, maxFuel);
            if (curTicks == maxTicks)
            {
                break;
            }
            if (neededFuel == maxFuel)
            {
                break;
            }

            int64_t bestTick = -1;
            int64_t bestDir = -1;
            int64_t bestTicks = curTicks;

            for (int64_t tick = 0; tick < curTicks; tick++)
            {
                if (accels[tick] != Vec())
                {
                    continue;
                }
                Vec oldAccel = accels[tick];
                for (int64_t dir = 0; dir < 8; dir++)
                {
                    accels[tick] = dirs[dir];
                    int64_t tmpTicks = check();
                    if (tmpTicks > bestTicks)
                    {
                        bestTick = tick;
                        bestDir = dir;
                        bestTicks = tmpTicks;
                    }
                }
                accels[tick] = oldAccel;
            }
            if (bestTicks == curTicks)
            {
                printf("No improvement\n");
                break;
            }
            if (accels[bestTick] != Vec())
            {
                neededFuel--;
            }
            accels[bestTick] = dirs[bestDir];
            if (accels[bestTick] != Vec())
            {
                neededFuel++;
            }
        }

        //printf("Ticks: %" PRIi64 " of %" PRIi64 "  Fuel: %" PRIi64 " of %" PRIi64 "\n", curTicks, maxTicks, neededFuel, maxFuel);
        *pAccels = std::move(accels);
    }
}
