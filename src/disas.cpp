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

using std::string;
using std::vector;
using std::unique_ptr;

void usage(FILE* f)
{
    fprintf(f, "Usage: disas [<options>] [<infile>]\n");
    fprintf(f, "  <infile>\n");
    fprintf(f, "        Input file (default: stdin)\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
}

int main(int argc, char *argv[])
{
    bool gotInFile = false;

    bool help = false;
    string inFile;

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

    uint32_t numEntries = code.m_entries.size() - 1;
    for (uint32_t iEntry = 0; iEntry < numEntries; iEntry++)
    {
        printf("%08" PRIx32 ":\n", iEntry);
        uint32_t iStartInst = code.m_entries[iEntry].m_iInst;
        uint32_t iEndInst = code.m_entries[iEntry + 1].m_iInst;
        for (uint32_t iInst = iStartInst; iInst < iEndInst; iInst++)
        {
            printf("    %08" PRIx32 ": ", iInst);
            auto& inst = code.m_insts[iInst];
            switch (inst.m_instType)
            {
            case InstType::Invalid:
                printf("Invalid");
                break;
            case InstType::Call:
                printf("Call");
                break;
            case InstType::TailCall:
                printf("TailCall");
                break;
            case InstType::Lambda:
                printf("Lambda %08" PRIx32 "", inst.m_lambdaData.m_iEntry);
                break;
            case InstType::Jf:
                printf("Jf %08" PRIx32 "", inst.m_jfData.m_iInst);
                break;
            case InstType::Jmp:
                printf("Jmp %08" PRIx32 "", inst.m_jmpData.m_iInst);
                break;
            case InstType::Integer:
                printf("Integer %s", Int::format(inst.m_integerData.m_value).c_str());
                break;
            case InstType::Boolean:
                printf("Boolean %s", inst.m_booleanData.m_value ? "true" : "false");
                break;
            case InstType::Variable:
                printf("Variable %" PRIi32 "", inst.m_variableData.m_slot);
                break;
            case InstType::Return:
                printf("Return");
                break;
            case InstType::Inc: printf("Inc"); break;
            case InstType::Dec: printf("Dec"); break;
            case InstType::Neg: printf("Neg"); break;
            case InstType::Not: printf("Not"); break;
            case InstType::Add: printf("Add"); break;
            case InstType::Sub: printf("Sub"); break;
            case InstType::Mul: printf("Mul"); break;
            case InstType::Div: printf("Div"); break;
            case InstType::Eq: printf("Eq"); break;
            case InstType::Ne: printf("Ne"); break;
            case InstType::Lt: printf("Lt"); break;
            case InstType::Gt: printf("Gt"); break;
            case InstType::Le: printf("Le"); break;
            case InstType::Ge: printf("Ge"); break;
            }
            printf("\n");
        }
        printf("\n");
    }

    return 0;
}
