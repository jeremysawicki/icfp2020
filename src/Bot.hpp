#ifndef BOT_HPP
#define BOT_HPP

#include "Common.hpp"
#include "Game.hpp"

class Bot
{
public:
    Bot();
    virtual ~Bot() = 0;

    virtual void getParams(const Info& info,
                           Params* pParams);

    virtual void getCommands(const Info& info,
                             const State& state,
                             std::vector<Command>* pCommands);
};

#endif
