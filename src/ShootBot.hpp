#ifndef SHOOTBOT_HPP
#define SHOOTBOT_HPP

#include "Common.hpp"
#include "Bot.hpp"

class ShootBot : public Bot
{
public:
    ShootBot();
    virtual ~ShootBot() override;

    virtual void getCommands(const Info& info,
                             const State& state,
                             std::vector<Command>* pCommands) override;
};

#endif
