#include "Common.hpp"
#include "ParseUtils.hpp"
#include "Protocol.hpp"
#include "Cleanup.hpp"
#include "Game.hpp"
#include "Bot.hpp"
#include "BotFactory.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

const char* defaultBotName = "shoot";

void usage(FILE* f)
{
    fprintf(f, "Usage: tutorial [<options>] <tutorial number>\n");
    fprintf(f, "  <tutorial number>\n");
    fprintf(f, "        Tutorial number (1-13)\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
    fprintf(f, "  -b <bot>\n");
    fprintf(f, "        Use the specified bot (default: %s)\n", defaultBotName);
    fprintf(f, "Bots:\n");
    vector<string> nameList = BotFactory::getList();
    for (auto& name : nameList)
    {
        fprintf(f, "  %s\n", name.c_str());
    }
    std::vector<std::string> getList();
}

int main(int argc, char *argv[])
{
    bool gotBotName = false;
    bool gotTutorialNum = false;

    bool help = false;
    string botName;
    int64_t tutorialNum;

    int iArg = 1;
    while (iArg < argc)
    {
        string strArg = argv[iArg++];

        if (strArg == "-h" || strArg == "--help")
        {
            help = true;
        }
        else if (strArg == "-b")
        {
            if (iArg >= argc)
            {
                usage(stderr);
                return 1;
            }
            strArg = argv[iArg++];
            botName = strArg;
            gotBotName = true;
        }
        else if (!gotTutorialNum)
        {
            if (!parseI64(strArg, &tutorialNum))
            {
                usage(stderr);
                return 1;
            }
            gotTutorialNum = true;
        }
        else
        {
            usage(stderr);
            return 1;
        }
    }

    if (help)
    {
        usage(stdout);
        return 0;
    }

    if (!gotTutorialNum)
    {
        usage(stderr);
        return 1;
    }

    if (!gotBotName)
    {
        botName = defaultBotName;
    }

    unique_ptr<Bot> pBot(BotFactory::create(botName));
    if (!pBot)
    {
        usage(stderr);
        return 1;
    }

    string msg;

    Protocol::init();
    Cleanup cleanupProtocol([](){ Protocol::cleanup(); });

    bool verbose = true;
    if (!Protocol::initAPIKey(verbose, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    Role role;
    int64_t playerKey;
    if (!Protocol::createTutorial(tutorialNum, &role, &playerKey, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    Info info;
    if (!Protocol::join(playerKey, &info, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    if (info.m_stage == Stage::After)
    {
        return 0;
    }

    State state;
    if (!Protocol::startTutorial(playerKey, &info, &state, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    while (info.m_stage != Stage::After)
    {
        vector<Command> commands;
        pBot->getCommands(info, state, &commands);

        if (!Protocol::play(playerKey, commands, &info, &state, &msg))
        {
            fprintf(stderr, "%s\n", msg.c_str());
            return 1;
        }
    }

    if (!Protocol::getResult(playerKey, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    return 0;
}
