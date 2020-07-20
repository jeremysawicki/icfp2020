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
    int64_t param4 = std::max(cost/(4*2), (int64_t)1);
    cost -= param4 * 2;
    int64_t param3 = cost/(3*12);
    cost -= param3 * 12;
    int64_t param2 = cost/(2*4);
    cost -= param2 * 4;
    int64_t param1 = cost/(1*1);
    cost -= param1 * 1;

    *pParams = {param1, param2, param3, param4};
}

void Bot::getCommands(const Info& info,
                      const State& state,
                      vector<Command>* pCommands)
{
}
