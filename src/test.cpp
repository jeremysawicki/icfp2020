#include "Common.hpp"
#include "Protocol.hpp"
#include "Cleanup.hpp"
#include "Game.hpp"

using std::string;
using std::vector;

bool test(string* pMsg)
{
    int64_t tutorialNum = 6;
    Role role;
    int64_t playerKey;
    if (!Protocol::createTutorial(tutorialNum, &role, &playerKey, pMsg))
    {
        return false;
    }

    Info info;
    if (!Protocol::join(playerKey, &info, pMsg))
    {
        return false;
    }

    State state;
    if (!Protocol::startTutorial(playerKey, &info, &state, pMsg))
    {
        return false;
    }

    return true;
    vector<Command> commands;
    auto& command = commands.emplace_back();
    command.m_commandType = CommandType::Shoot;
    command.m_id = 0;
    command.m_vec = {48, 0};
    command.m_val = 64+11;

    if (!Protocol::play(playerKey, commands, &info, &state, pMsg))
    {
        return false;
    }

    if (!Protocol::play(playerKey, commands, &info, &state, pMsg))
    {
        return false;
    }

    if (!Protocol::play(playerKey, commands, &info, &state, pMsg))
    {
        return false;
    }

    command.m_val = 44;
    if (!Protocol::play(playerKey, commands, &info, &state, pMsg))
    {
        return false;
    }

    if (!Protocol::getResult(playerKey, pMsg))
    {
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    string msg;

    Protocol::init();
    Cleanup cleanupProtocol([](){ Protocol::cleanup(); });

    if (!Protocol::initAPIKey(true, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    if (!test(&msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    return 0;
}
