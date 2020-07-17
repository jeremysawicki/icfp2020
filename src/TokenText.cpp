#include "TokenText.hpp"
#include "Token.hpp"
#include "Function.hpp"
#include "StringUtils.hpp"
#include <unordered_map>

using std::string;
using std::vector;
using std::unordered_map;
using std::pair;

namespace
{
    enum class ParseState : uint32_t
    {
        BeforeToken,
        AfterToken,
        InToken
    };

    constexpr pair<const char*, Function> funcIndex[] =
    {
        { "inc", Function::Inc },
        { "dec", Function::Dec },
        { "neg", Function::Neg },
        { "not", Function::Not },
        { "add", Function::Add },
        { "sub", Function::Sub },
        { "mul", Function::Mul },
        { "div", Function::Div },
        { "eq", Function::Eq },
        { "ne", Function::Ne },
        { "lt", Function::Lt },
        { "gt", Function::Gt },
        { "le", Function::Le },
        { "ge", Function::Ge }
    };

    bool parseFunction(const string& str, Function* pFunc)
    {
        for (auto& entry : funcIndex)
        {
            if (strcmp(str.c_str(), entry.first) == 0)
            {
                if (pFunc) *pFunc = entry.second;
                return true;
            }
        }
        return false;
    }

    bool formatFunction(Function func, string* pStr)
    {
        for (auto& entry : funcIndex)
        {
            if (entry.second == func)
            {
                if (pStr) *pStr = entry.first;
                return true;
            }
        }
        return false;
    }
}

bool parseTokenText(const string& text,
                    vector<Token>* pTokens,
                    string* pMsg)
{
    ParseState state = ParseState::BeforeToken;
    string str;
    vector<Token> tokens;
    unordered_map<string, uint32_t> varNameToIdx;
    vector<string> varIdxToName;
    unordered_map<uint32_t, uint32_t> varIdToIdx;
    vector<uint32_t> varIdxToId;

    size_t size = text.size();
    for (size_t pos = 0; pos <= size; pos++)
    {
        char ch = (pos == size) ? '\0' : text[pos];
        bool isWhite =
            ch == ' ' ||
            ch == '\t' ||
            ch == '\r' ||
            ch == '\n' ||
            ch == '\v';
        bool isNormal =
            (ch >= 33 && ch <= 126) &&
            ch != '\\' &&
            ch != '$' &&
            ch != '(' &&
            ch != ')';

        if (state == ParseState::InToken)
        {
            if (isNormal)
            {
                str.push_back(ch);
                continue;
            }

            Function func = Function::Invalid;
            bool intInRange = false;
            Int intValue;
            if (parseFunction(str, &func))
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Function);
                token.m_functionData.m_func = func;
            }
            else if (str == "ap")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Apply);
            }
            else if (str == "lambda")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Lambda);
            }
            else if (str == "if")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::If);
            }
            else if (str == "and")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::And);
            }
            else if (str == "or")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Or);
            }
            else if (str == "let")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Let);
            }
            else if (str == "=")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Assign);
            }
            else if (str == "false")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Boolean);
                token.m_booleanData.m_value = false;
            }
            else if (str == "true")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Boolean);
                token.m_booleanData.m_value = true;
            }
            else if (Int::parse(str, &intInRange, &intValue))
            {
                if (!intInRange)
                {
                    if (pMsg) *pMsg = "Integer literal overflow";
                    return false;
                }

                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Integer);
                token.m_integerData.m_value = std::move(intValue);
            }
            else
            {
                auto findIt = varNameToIdx.find(str);
                uint32_t varId;
                if (findIt != varNameToIdx.end())
                {
                    varId = findIt->second;
                }
                else
                {
                    varId = varIdxToName.size();
                    varIdxToName.push_back(str);
                    varNameToIdx.emplace(str, varId);
                }

                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Variable);
                token.m_variableData.m_varId = varId;
            }

            str.clear();
            state = ParseState::AfterToken;
        }

        if (pos == size)
        {
            break;
        }

        if (ch == '\\')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Lambda);
            state = ParseState::BeforeToken;
            continue;
        }

        if (ch == '$')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Apply);
            state = ParseState::BeforeToken;
            continue;
        }

        if (ch == '(')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::LParen);
            state = ParseState::BeforeToken;
            continue;
        }

        if (ch == ')')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::RParen);
            state = ParseState::BeforeToken;
            continue;
        }

        if (state == ParseState::BeforeToken && isNormal)
        {
            str.push_back(ch);
            state = ParseState::InToken;
            continue;
        }

        if (isWhite)
        {
            state = ParseState::BeforeToken;
            continue;
        }

        if (pMsg) *pMsg = "Invalid character";
        return false;
    }

    if (pTokens) *pTokens = std::move(tokens);
    return true;
}

bool formatTokenText(const vector<Token>& tokens,
                     string* pText,
                     string* pMsg)
{
    string text;
    bool needSpace = false;
    for (const Token& token : tokens)
    {
        TokenType tokenType = token.getTokenType();

        if (tokenType == TokenType::RParen)
        {
            needSpace = false;
        }

        if (needSpace)
        {
            text += ' ';
        }

        switch (token.getTokenType())
        {
        case TokenType::Apply:
        {
            text += "ap";
            break;
        }
        case TokenType::Lambda:
        {
            text += '\\';
            break;
        }
        case TokenType::If:
        {
            text += "if";
            break;
        }
        case TokenType::And:
        {
            text += "and";
            break;
        }
        case TokenType::Or:
        {
            text += "or";
            break;
        }
        case TokenType::Integer:
        {
            text += Int::format(token.m_integerData.m_value);
            break;
        }
        case TokenType::Boolean:
        {
            text += token.m_booleanData.m_value ? "true" : "false";
            break;
        }
        case TokenType::Variable:
        {
            text += 'x';
            text += strprintf("%" PRIu32 "", token.m_variableData.m_varId);
            break;
        }
        case TokenType::Function:
        {
            string str;
            if (!formatFunction(token.m_functionData.m_func, &str))
            {
                if (pMsg) *pMsg = "Unexpected function";
                return false;
            }
            text += str;
            break;
        }
        case TokenType::Let:
        {
            text += "let";
            break;
        }
        case TokenType::Assign:
        {
            text += "=";
            break;
        }
        case TokenType::LParen:
        {
            text += "(";
            break;
        }
        case TokenType::RParen:
        {
            text += ")";
            break;
        }
        default:
        {
            if (pMsg) *pMsg = "Unexpected token type";
            return false;
        }
        }

        needSpace = (tokenType != TokenType::LParen);
    }

    if (pText) *pText = text;
    return true;
}
