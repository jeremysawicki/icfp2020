#ifndef GAME_HPP
#define GAME_HPP

#include "Common.hpp"

enum class Role : uint32_t
{
    Attacker = 0,
    Defender = 1
};

enum class Stage : uint32_t
{
    Before = 0,
    During = 1,
    After = 2
};

class Vec
{
public:
    int64_t m_x = 0;
    int64_t m_y = 0;
};

class Params
{
public:
    int64_t m_param1 = 0;
    int64_t m_param2 = 0;
    int64_t m_param3 = 0;
    int64_t m_param4 = 0;
};

enum class CommandType : uint32_t
{
    Accelerate = 0,
    Detonate = 1,
    Shoot = 2
};

class Command
{
public:
    CommandType m_commandType = CommandType::Accelerate;
    int64_t m_id = 0;
    Vec m_vec;
    int64_t m_val = 0;
};

class Info
{
public:
    Stage m_stage = Stage::Before;
    int64_t m_maxTicks = 0;
    int64_t m_id = 0;
    int64_t m_val1 = 0;
    int64_t m_val2 = 0;
    int64_t m_val3 = 0;
};

class Effect
{
public:
    CommandType m_commandType = CommandType::Accelerate;
};

class Ship
{
public:
    Role m_role = Role::Attacker;
    int64_t m_id = 0;
    Vec m_pos;
    Vec m_vel;
    Params m_params;
    int64_t m_val5 = 0;
    int64_t m_val6 = 0;
    int64_t m_val7 = 0;
    std::vector<Effect> m_effects;
};

class State
{
public:
    int64_t m_tick = 0;
    std::vector<Ship> m_ships;
};

#endif
