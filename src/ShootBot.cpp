#include "ShootBot.hpp"

using std::vector;

ShootBot::ShootBot()
{
}

ShootBot::~ShootBot()
{
}

void ShootBot::getCommands(const Info& info,
                           const State& state,
                           vector<Command>* pCommands)
{
    Role role = info.m_role;

    for (auto& self : state.m_ships)
    {
        if (self.m_role != role) continue;

        if (state.m_tick < 10)
        {
            auto& command = pCommands->emplace_back();
            command.m_commandType = CommandType::Accelerate;
            command.m_id = self.m_id;
            if (abs(self.m_pos.m_x) < abs(self.m_pos.m_y))
            {
                command.m_vec = {1, 0};
            }
            else
            {
                command.m_vec = {0, 1};
            }
        }

        if (role == Role::Attacker)
        {
            int64_t param1 = self.m_params.m_param1;
            if (param1 <= 0)
            {
                return;
            }
            int64_t val = param1 / 8;
            if (val < 0) val = 0;
            if (val > param1) val = param1;

            for (auto& other : state.m_ships)
            {
                if (other.m_role == role) continue;

                auto& shootCommand = pCommands->emplace_back();
                shootCommand.m_commandType = CommandType::Shoot;
                shootCommand.m_id = self.m_id;
                shootCommand.m_vec.m_x = other.m_pos.m_x + other.m_vel.m_x;
                shootCommand.m_vec.m_y = other.m_pos.m_y + other.m_vel.m_y;
                shootCommand.m_val = val;

                break;
            }
        }
    }
}
