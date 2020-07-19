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
    *pParams = {0, 0, 0, 1};
}

void Bot::getCommands(const Info& info,
                      const State& state,
                      vector<Command>* pCommands)
{
}
