#include "Common.hpp"
#include "FileUtils.hpp"
#include "Token.hpp"
#include "TokenText.hpp"
#include "Expr.hpp"
#include "ParseExpr.hpp"
#include "DesugarExpr.hpp"
#include "FormatExpr.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

void usage(FILE* f)
{
    fprintf(f, "Usage: desugar [<options>] [<infile>] [<outfile>]\n");
    fprintf(f, "  <infile>\n");
    fprintf(f, "        Input file (default: stdin)\n");
    fprintf(f, "  <outfile>\n");
    fprintf(f, "        Output file (default: stdout)\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
}
int main(int argc, char *argv[])
{
    bool gotInFile = false;
    bool gotOutFile = false;

    bool help = false;
    string inFile;
    string outFile;

    int iArg = 1;
    while (iArg < argc)
    {
        string strArg = argv[iArg++];

        if (strArg == "-h" || strArg == "--help")
        {
            help = true;
        }
        else if (!gotInFile)
        {
            inFile = strArg;
            gotInFile = true;
        }
        else if (!gotOutFile)
        {
            outFile = strArg;
            gotOutFile = true;
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

    if (gotInFile && inFile == "-")
    {
        gotInFile = false;
    }
    if (gotOutFile && outFile == "-")
    {
        gotOutFile = false;
    }

    string msg;

    string text;
    if (!(gotInFile ?
          readFile(inFile, &text) :
          readFile(stdin, &text)))
    {
        fprintf(stderr, "Error reading input\n");
        return 1;
    }

    vector<Token> tokens;
    if (!parseTokenText(text, &tokens, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    unique_ptr<Expr> pExpr;
    if (!parseExpr(tokens, &pExpr, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    if (!desugarExpr(pExpr, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }
    
    vector<Token> formattedTokens;
    if (!formatExpr(pExpr, &formattedTokens, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    string formattedText;
    if (!formatTokenText(formattedTokens, &formattedText, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    formattedText.push_back('\n');

    if (!(gotOutFile ?
          writeFile(outFile, formattedText) :
          writeFile(stdout, formattedText)))
    {
        fprintf(stderr, "Error writing output\n");
        return 1;
    }

    return 0;
}
