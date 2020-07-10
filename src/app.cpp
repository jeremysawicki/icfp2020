#include "Common.hpp"
#include "Protocol.hpp"
#include "Cleanup.hpp"

using std::string;

void usage(FILE* f)
{
    fprintf(f, "Usage: app [<options>] <url> <player key>\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
    fprintf(f, "  -v    Verbose: print HTTP response\n");
}

int main(int argc, char *argv[])
{
    bool gotUrl = false;
    bool gotPlayerKey = false;

    bool help = false;
    bool verbose = false;
    string url;
    string playerKey;

    int iArg = 1;
    while (iArg < argc)
    {
        string strArg = argv[iArg++];

        if (gotUrl && !gotPlayerKey)
        {
            playerKey = strArg;
            gotPlayerKey = true;
        }
        else if (strArg == "-h" || strArg == "--help")
        {
            help = true;
        }
        else if (strArg == "-v")
        {
            verbose = true;
        }
        else if (!gotUrl)
        {
            url = strArg;
            gotUrl = true;
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

    if (!gotUrl ||
        !gotPlayerKey)
    {
        usage(stderr);
        return 1;
    }

    printf("ServerUrl: %s; PlayerKey: %s\n", url.c_str(), playerKey.c_str());
    fflush(stdout);

    Protocol::init();
    Cleanup cleanupProtocol([](){ Protocol::cleanup(); });

    string msg;
    string response;
    if (!Protocol::test(url,
                        playerKey,
                        &response,
                        &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    fprintf(stderr, "Success\n");

    if (verbose)
    {
        printf("%s\n", response.c_str());
    }

    return 0;
}
