#ifndef CLONEBOT_HPP
#define CLONEBOT_HPP

#include "Common.hpp"
#include "Bot.hpp"
#include "ShootBot.hpp"

class CloneBot : public ShootBot
{
public:
    CloneBot();
    virtual ~CloneBot() override;

    virtual void getParams(const Info& info,
                           Params* pParams) override;

    virtual void getCommands(const Info& info,
                             const State& state,
                             std::vector<Command>* pCommands) override;
};

#endif
