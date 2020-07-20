#include "OrbitBot.hpp"
#include "Gravity.hpp"

using std::vector;

OrbitBot::OrbitBot()
{
}

OrbitBot::~OrbitBot()
{
}

void OrbitBot::getCommands(const Info& info,
                           const State& state,
                           vector<Command>* pCommands)
{
    Role role = info.m_role;
    bool haveGravity = info.m_minRadius != -1;

    //if (state.m_tick == 0)
    {
        m_expectedPos.resize(state.m_ships.size());
        m_expectedVel.resize(state.m_ships.size());
        if (haveGravity)
        {
            m_accels.resize(state.m_ships.size());
        }
    }

    for (auto& self : state.m_ships)
    {
        if (m_expectedPos[self.m_id] != Vec() ||
            m_expectedVel[self.m_id] != Vec())
        {
            if (self.m_pos != m_expectedPos[self.m_id] ||
                self.m_vel != m_expectedVel[self.m_id])
            {
                printf("Position/velocity MISMATCH!\n");
            }
            else
            {
                printf("Position/velocity ok\n");
            }
        }

        Vec accel;
        if (self.m_role == role)
        {
            if (haveGravity)
            {
                if (m_accels[self.m_id].empty())
                {
                    Gravity::solve(info.m_minRadius,
                                   info.m_maxRadius,
                                   self.m_pos,
                                   self.m_vel,
                                   state.m_tick,
                                   info.m_maxTicks,
                                   self.m_params.m_fuel,
                                   &m_accels[self.m_id]);
                }
                accel = m_accels[self.m_id][state.m_tick];
            }
            else if (state.m_tick < 10)
            {
                if (abs(self.m_pos.m_x) < abs(self.m_pos.m_y))
                {
                    accel = {1, 0};
                }
                else
                {
                    accel = {0, 1};
                }
            }

            if (self.m_params.m_fuel == 0)
            {
                accel = Vec();
            }

            if (accel != Vec())
            {
                auto& command = pCommands->emplace_back();
                command.m_commandType = CommandType::Accelerate;
                command.m_id = self.m_id;
                command.m_vec = accel;
            }
        }

        Gravity::step(haveGravity,
                      self.m_pos,
                      self.m_vel,
                      accel,
                      &m_expectedPos[self.m_id],
                      &m_expectedVel[self.m_id]);
    }
}
