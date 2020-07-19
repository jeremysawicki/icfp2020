#include "Common.hpp"
#include "Protocol.hpp"
#include "Cleanup.hpp"
#include "Game.hpp"

using std::string;

bool test(string* pMsg)
{
    int64_t tutorialNum = 2;
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
