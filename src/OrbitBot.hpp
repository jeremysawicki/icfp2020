#ifndef ORBITBOT_HPP
#define ORBITBOT_HPP

#include "Common.hpp"
#include "Bot.hpp"

class OrbitBot : public Bot
{
public:
    OrbitBot();
    virtual ~OrbitBot() override;

    virtual void getCommands(const Info& info,
                             const State& state,
                             std::vector<Command>* pCommands) override;

    std::vector<Vec> m_expectedPos;
    std::vector<Vec> m_expectedVel;
    std::vector<std::vector<Vec>> m_accels;
};

#endif
