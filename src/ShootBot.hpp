#ifndef SHOOTBOT_HPP
#define SHOOTBOT_HPP

#include "Common.hpp"
#include "Bot.hpp"
#include "OrbitBot.hpp"

class ShootBot : public OrbitBot
{
public:
    ShootBot();
    virtual ~ShootBot() override;

    virtual void getParams(const Info& info,
                           Params* pParams) override;

    virtual void getCommands(const Info& info,
                             const State& state,
                             std::vector<Command>* pCommands) override;
};

#endif
