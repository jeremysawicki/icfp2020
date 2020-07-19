#ifndef PASSBOT_HPP
#define PASSBOT_HPP

#include "Common.hpp"
#include "Bot.hpp"

class PassBot : public Bot
{
public:
    PassBot();
    virtual ~PassBot() override;

    virtual void getParams(const Info& info,
                           Params* pParams) override;
};

#endif
