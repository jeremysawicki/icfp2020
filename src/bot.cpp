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

const char* defaultBotName = "pass";

void usage(FILE* f)
{
    fprintf(f, "Usage: bot [<options>] <player key>\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
    fprintf(f, "  -b <bot>\n");
    fprintf(f, "        Use the specified bot (default: %s)\n", defaultBotName);
    fprintf(f, "  -d <url>\n");
    fprintf(f, "        Run in docker mode with the supplied URL\n");
}

int main(int argc, char *argv[])
{
    bool gotBotName = false;
    bool gotUrl = false;
    bool gotPlayerKey = false;

    bool help = false;
    string botName;
    string url;
    int64_t playerKey;

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
        else if (strArg == "-d")
        {
            if (iArg >= argc)
            {
                usage(stderr);
                return 1;
            }
            strArg = argv[iArg++];
            url = strArg;
            gotUrl = true;
        }
        else if (!gotPlayerKey)
        {
            if (!parseI64(strArg, &playerKey))
            {
                usage(stderr);
                return 1;
            }
            gotPlayerKey = true;
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

    if (!gotPlayerKey)
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

    if (gotUrl)
    {
        printf("url = %s\n", url.c_str());
    }
    printf("player key = %" PRIi64 "\n", playerKey);

    string msg;

    Protocol::init();
    Cleanup cleanupProtocol([](){ Protocol::cleanup(); });

    bool verbose = true;
    if (gotUrl)
    {
        if (!Protocol::initDocker(url, verbose, &msg))
        {
            fprintf(stderr, "%s\n", msg.c_str());
            return 1;
        }
    }
    else
    {
        if (!Protocol::initAPIKey(verbose, &msg))
        {
            fprintf(stderr, "%s\n", msg.c_str());
            return 1;
        }
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

    Params params;
    pBot->getParams(info, &params);

    State state;
    if (!Protocol::start(playerKey, params, &info, &state, &msg))
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

    return 0;
}
