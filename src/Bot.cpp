#include "Bot.hpp"

using std::vector;

Bot::Bot()
{
}

Bot::~Bot()
{
}

void Bot::getParams(const Info& info,
                    Params* pParams)
{
    // costs: 1, 4, 12, 2
    int64_t cost = info.m_maxCost;

    int64_t ships = 1; //std::max(cost/(4*2), (int64_t)1);
    cost -= ships * 2;

    int64_t cooling = cost/(10*12);
    cost -= cooling * 12;

    int64_t guns = cost/(9*4);
    cost -= guns * 4;

    int64_t fuel = cost/(1*1);
    cost -= fuel * 1;

    *pParams = {fuel, guns, cooling, ships};
}

void Bot::getCommands(const Info& info,
                      const State& state,
                      vector<Command>* pCommands)
{
}
