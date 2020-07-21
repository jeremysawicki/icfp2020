#include "CloneBot.hpp"

using std::vector;

CloneBot::CloneBot()
{
}

CloneBot::~CloneBot()
{
}

void CloneBot::getParams(const Info& info,
                         Params* pParams)
{
    if (info.m_role == Role::Attacker)
    {
        ShootBot::getParams(info, pParams);
        return;
    }

    // costs: 1, 4, 12, 2
    int64_t cost = info.m_maxCost;

    int64_t fuel = 0;
    int64_t guns = 0;
    int64_t cooling = 0;
    int64_t ships = 0;

    ships = 1;
    cost -= ships * 2;

    fuel = cost/(5*1);
    cost -= fuel * 1;

    cooling = cost/(3*12);
    cost -= cooling * 12;

    while (cost >= 10)
    {
        ships += 1;
        cost -= 1 * 2;

        fuel += 8;
        cost -= 8 * 1;
    }

    fuel += cost;
    cost -= cost * 1;

    *pParams = {fuel, guns, cooling, ships};
}

void CloneBot::getCommands(const Info& info,
                           const State& state,
                           vector<Command>* pCommands)
{
    ShootBot::getCommands(info, state, pCommands);

    Role role = info.m_role;
    if (role == Role::Attacker)
    {
        return;
    }

    for (auto& self : state.m_ships)
    {
        if (self.m_role != role) continue;

        if (self.m_params.m_ships > 1)
        {
            auto& command = pCommands->emplace_back();
            command.m_commandType = CommandType::Clone;
            command.m_id = self.m_id;
            command.m_params.m_fuel = 8;
            command.m_params.m_guns = 0;
            command.m_params.m_cooling = 0;
            command.m_params.m_ships = 1;
        }
    }
}
