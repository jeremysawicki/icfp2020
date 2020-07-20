#include "ShootBot.hpp"

using std::vector;

ShootBot::ShootBot()
{
}

ShootBot::~ShootBot()
{
}

void ShootBot::getParams(const Info& info,
                         Params* pParams)
{
    // costs: 1, 4, 12, 2
    int64_t cost = info.m_maxCost;

    int64_t ships = 1; //std::max(cost/(4*2), (int64_t)1);
    cost -= ships * 2;

    int64_t fuel = cost/(20*1);
    cost -= fuel * 1;

    int64_t cooling = cost/(4*12);
    cost -= cooling * 12;

    int64_t guns = cost/(1*4);
    cost -= guns * 4;

    *pParams = {fuel, guns, cooling, ships};
}

void ShootBot::getCommands(const Info& info,
                           const State& state,
                           vector<Command>* pCommands)
{
    OrbitBot::getCommands(info, state, pCommands);

    Role role = info.m_role;

    for (auto& self : state.m_ships)
    {
        if (self.m_role != role) continue;

        //if (role != Role::Attacker) continue;

        if (self.m_params.m_cooling > 0 && self.m_heat > 0)
        {
            continue;
        }

        int64_t guns = self.m_params.m_guns;
        int64_t limit = self.m_maxHeat - self.m_heat + self.m_params.m_cooling;
        int64_t val = std::min(guns, limit);

        if (val <= 0)
        {
            continue;
        }

        for (auto& other : state.m_ships)
        {
            if (other.m_role == role) continue;

            //int64_t selfX = self.m_pos.m_x + self.m_vel.m_x;
            //int64_t selfY = self.m_pos.m_y + self.m_vel.m_y;
            //int64_t otherX = other.m_pos.m_x + other.m_vel.m_x;
            //int64_t otherY = other.m_pos.m_y + other.m_vel.m_y;
            int64_t selfX = m_expectedPos[self.m_id].m_x;
            int64_t selfY = m_expectedPos[self.m_id].m_y;
            int64_t otherX = m_expectedPos[other.m_id].m_x;
            int64_t otherY = m_expectedPos[other.m_id].m_y;
            int64_t absDistX = abs(otherX - selfX);
            int64_t absDistY = abs(otherY - selfY);
            if (absDistX <= 4 && absDistY <= 4)
            {
                // Too close
                continue;
            }

            if (absDistX > 8 &&
                absDistY > 8 &&
                abs(absDistX - absDistY) > 8)
            {
                // Not aligned
                continue;
            }

            auto& shootCommand = pCommands->emplace_back();
            shootCommand.m_commandType = CommandType::Shoot;
            shootCommand.m_id = self.m_id;
            shootCommand.m_vec.m_x = otherX;
            shootCommand.m_vec.m_y = otherY;
            shootCommand.m_val = val;

            break;
        }
    }
}
