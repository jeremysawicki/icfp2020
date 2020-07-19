#include "Common.hpp"
#include "Protocol.hpp"
#include "Cleanup.hpp"
#include "FileUtils.hpp"
#include "Token.hpp"
#include "TokenText.hpp"
#include "SymTable.hpp"
#include "ParseValue.hpp"
#include "Bindings.hpp"
#include "Value.hpp"
#include "Eval.hpp"
#include "PrintValue.hpp"
#include "FormatValue.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

void usage(FILE* f)
{
    fprintf(f, "Usage: run [<options>] [<expression file>] [<arg>...]\n");
    fprintf(f, "  <expression file>\n");
    fprintf(f, "        Expression file (default: stdin)\n");
    fprintf(f, "  <arg>\n");
    fprintf(f, "        Function arguments\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
    fprintf(f, "  -b <bindings file>\n");
    fprintf(f, "        Load bindings from the specified file\n");
}

int main(int argc, char *argv[])
{
    bool gotExprFile = false;

    bool help = false;
    string exprFile;
    vector<string> bindingsFiles;
    vector<string> args;

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
            bindingsFiles.push_back(strArg);
        }
        else if (!gotExprFile)
        {
            exprFile = strArg;
            gotExprFile = true;
        }
        else
        {
            args.push_back(strArg);
        }
    }

    if (help)
    {
        usage(stdout);
        return 0;
    }

    if (gotExprFile && exprFile == "-")
    {
        gotExprFile = false;
    }

    string msg;

    Protocol::init();
    Cleanup cleanupProtocol([](){ Protocol::cleanup(); });

    if (!Protocol::initAPIKey(false, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    SymTable symTable;
    Bindings bindings;

    for (auto& bindingsFile : bindingsFiles)
    {
        string text;
        if (!readFile(bindingsFile, &text))
        {
            fprintf(stderr, "Error reading bindings\n");
            return 1;
        }

        vector<Token> tokens;
        if (!parseTokenText(symTable, text, &tokens, &msg))
        {
            fprintf(stderr, "%s\n", msg.c_str());
            return 1;
        }

        bindings.resize(symTable.size());

        if (!parseBindings(tokens, bindings, &msg))
        {
            fprintf(stderr, "%s\n", msg.c_str());
            return 1;
        }
    }

    string text;
    if (!(gotExprFile ?
          readFile(exprFile, &text) :
          readFile(stdin, &text)))
    {
        fprintf(stderr, "Error reading expression\n");
        return 1;
    }

    if (!args.empty())
    {
        text = '(' + text;
        for (auto& arg : args)
        {
            text += ' ';
            text += arg;
        }
        text += ')';
    }

    vector<Token> tokens;
    if (!parseTokenText(symTable, text, &tokens, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    //bindings.resize(symTable.size());

    Value value;
    if (!parseValue(tokens, bindings, value, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

#if 0
    if (!printValue(value, true, &msg))
    {
        printf("\n");
        printf("%s\n", msg.c_str());
        return 1;
    }
    printf("\n");
#endif

#if 1
    vector<Token> formattedTokens;
    if (!formatValue(value, &formattedTokens, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    string formattedText;
    if (!formatTokenText(symTable,
                         formattedTokens,
                         &formattedText,
                         &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    printf("%s\n", formattedText.c_str());
#endif

    return 0;
}
