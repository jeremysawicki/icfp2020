#include "Common.hpp"
#include "Protocol.hpp"
#include "Cleanup.hpp"

using std::string;

void usage(FILE* f)
{
    fprintf(f, "Usage: send [<options>] <request>\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
    fprintf(f, "  -v    Verbose: print extra information\n");
}

int main(int argc, char *argv[])
{
    bool gotRequest = false;

    bool help = false;
    bool verbose = false;
    string request;

    int iArg = 1;
    while (iArg < argc)
    {
        string strArg = argv[iArg++];

        if (strArg == "-h" || strArg == "--help")
        {
            help = true;
        }
        else if (strArg == "-v")
        {
            verbose = true;
        }
        else if (!gotRequest)
        {
            request = strArg;
            gotRequest = true;
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

    if (!gotRequest)
    {
        usage(stderr);
        return 1;
    }

    if (verbose)
    {
        fprintf(stderr, "Request size: %" PRIuZ "\n", request.size());
        fprintf(stderr, "Request:\n");
        fprintf(stderr, "%s\n", request.c_str());
    }

    Protocol::init();
    Cleanup cleanupProtocol([](){ Protocol::cleanup(); });

    string msg;
    string response;
    if (!Protocol::send(request,
                        &response,
                        &msg))
    {
        fprintf(stderr, "Request failed\n");
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    if (verbose)
    {
        fprintf(stderr, "Request succeeded\n");
        fprintf(stderr, "Response size: %" PRIuZ "\n", response.size());
        fprintf(stderr, "Response:\n");
    }

    printf("%s\n", response.c_str());

    return 0;
}
