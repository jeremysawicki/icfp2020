#include "Common.hpp"
#include "Protocol.hpp"
#include "Game.hpp"
#include "Cleanup.hpp"

using std::string;

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

    int64_t playerKey[2];
    if (!Protocol::create(&playerKey[0], &playerKey[1], &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    printf("0: %" PRIi64"\n", playerKey[0]);
    printf("1: %" PRIi64"\n", playerKey[1]);

    return 0;
}
