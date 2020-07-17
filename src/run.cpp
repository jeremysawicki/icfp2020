#include "Common.hpp"
#include "FileUtils.hpp"
#include "Token.hpp"
#include "TokenText.hpp"
#include "Expr.hpp"
#include "ParseExpr.hpp"
#include "DesugarExpr.hpp"
#include "OptimizeExpr.hpp"
#include "Code.hpp"
#include "ParseCode.hpp"
#include "Value.hpp"
#include "EvalCode.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

void usage(FILE* f)
{
    fprintf(f, "Usage: run [<options>] [<infile>] [<arg>...]\n");
    fprintf(f, "  <infile>\n");
    fprintf(f, "        Input file (default: stdin)\n");
    fprintf(f, "  <arg>\n");
    fprintf(f, "        Function arguments\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
}

int main(int argc, char *argv[])
{
    bool gotInFile = false;

    bool help = false;
    string inFile;
    vector<string> args;

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

    if (gotInFile && inFile == "-")
    {
        gotInFile = false;
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

    if (!optimizeExpr(pExpr, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    Code code;
    if (!parseCode(pExpr, &code, &msg))
    {
        printf("%s\n", msg.c_str());
        return 1;
    }

    Value value;
    if (!evalCode(code, &value, &msg))
    {
        printf("%s\n", msg.c_str());
        return 1;
    }

    if (value.m_valueType == ValueType::Integer)
    {
        printf("%s\n", Int::format(value.m_integerData.m_value).c_str());
    }
    else if (value.m_valueType == ValueType::Boolean)
    {
        printf("%s\n", value.m_booleanData.m_value ? "true" : "false");
    }
    else if (value.m_valueType == ValueType::Closure)
    {
        printf("<closure>\n");
    }
    else
    {
        printf("<unknown type>\n");
    }

    return 0;
}
