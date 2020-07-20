#include "Gravity.hpp"
#include "Xoshiro.hpp"
#include <random>

typedef Xoshiro128StarStar Gen;

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
               int64_t startTick,
               int64_t maxTicks,
               int64_t maxFuel,
               vector<Vec>* pAccels)
    {
        std::seed_seq seq{12345, 1};
        Gen gen(seq);

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

        auto check = [&](const vector<Vec>& accels) -> int64_t
        {
            Vec pos = startPos;
            Vec vel = startVel;
            for (int64_t tick = startTick; tick < maxTicks; tick++)
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

        vector<vector<Vec>> oldAccels;

        while (true)
        {
            curTicks = check(accels);
            printf("Ticks: %" PRIi64 " of %" PRIi64 "  Fuel: %" PRIi64 " of %" PRIi64 "\n", curTicks, maxTicks, neededFuel, maxFuel);
            if (curTicks == maxTicks)
            {
                break;
            }
            if (neededFuel == maxFuel)
            {
                break;
            }

            oldAccels.push_back(accels);

            int64_t bestTick = -1;
            int64_t bestDir = -1;
            int64_t bestBurst = -1;
            int64_t bestTicks = curTicks;
            uint32_t bestCount = 0;

            for (int64_t burst = 1; burst <= 8; burst *= 2)
            {
                if (oldAccels.size() < (size_t)burst)
                {
                    break;
                }

                auto& curOldAccels = oldAccels[oldAccels.size() - burst];
                auto curAccels = curOldAccels;

                for (int64_t tick = startTick; tick < curTicks; tick++)
                {
                    /*
                    if (accels[tick] != Vec())
                    {
                        continue;
                    }
                    */
                    //Vec oldAccel = accels[tick];
                    for (int64_t dir = 0; dir < 8; dir++)
                    {
                        for (int64_t burstTick = tick; burstTick < tick + burst && burstTick < maxTicks; burstTick++)
                        {
                            curAccels[burstTick] = dirs[dir];
                        }
                        //accels[tick] = dirs[dir];
                        int64_t tmpTicks = check(curAccels);
                        if (tmpTicks <= curTicks)
                        {
                            continue;
                        }
                        if (tmpTicks > bestTicks)
                        {
                            bestTick = tick;
                            bestDir = dir;
                            bestBurst = burst;
                            bestTicks = tmpTicks;
                            bestCount = 1;
                        }
                        else if (tmpTicks == bestTicks)
                        {
                            bestCount++;
                            if (gen() % bestCount == 0)
                            {
                                bestTick = tick;
                                bestDir = dir;
                                bestBurst = burst;
                                bestTicks = tmpTicks;
                            }
                        }
                    }

                    for (int64_t burstTick = tick; burstTick < tick + burst && burstTick < maxTicks; burstTick++)
                    {
                        curAccels[burstTick] = curOldAccels[burstTick];
                    }
                    //accels[tick] = oldAccel;
                }
            }
            printf("bestTicks = %" PRIi64 ", bestBurst = %" PRIi64 "\n", bestTicks, bestBurst);
            if (bestTicks == curTicks)
            {
                printf("No improvement\n");
                break;
            }
            auto& bestOldAccels = oldAccels[oldAccels.size() - bestBurst];
            for (int64_t tick = startTick; tick < maxTicks; tick++)
            {
                if (accels[tick] != Vec())
                {
                    neededFuel--;
                }
                accels[tick] = bestOldAccels[tick];
                if (accels[tick] != Vec())
                {
                    neededFuel++;
                }
            }
            for (int64_t burstTick = bestTick; burstTick < bestTick + bestBurst && burstTick < maxTicks; burstTick++)
            {
                if (accels[burstTick] != Vec())
                {
                    neededFuel--;
                }
                accels[burstTick] = dirs[bestDir];
                if (accels[burstTick] != Vec())
                {
                    neededFuel++;
                }
            }
        }

        //printf("Ticks: %" PRIi64 " of %" PRIi64 "  Fuel: %" PRIi64 " of %" PRIi64 "\n", curTicks, maxTicks, neededFuel, maxFuel);
        *pAccels = std::move(accels);
    }
}
