#include "TokenText.hpp"
#include "Token.hpp"
#include "Function.hpp"
#include "StringUtils.hpp"
#include "SymTable.hpp"

using std::string;
using std::vector;
using std::pair;

namespace
{
    enum class ParseState : uint32_t
    {
        BeforeToken,
        AfterToken,
        InToken,
        InSignal
    };

    constexpr pair<const char*, Function> funcIndex[] =
    {
        { "inc", Function::Inc },
        { "dec", Function::Dec },
        { "add", Function::Add },
        { "mul", Function::Mul },
        { "div", Function::Div },
        { "eq", Function::Eq },
        { "lt", Function::Lt },
        { "mod", Function::Modulate },
        { "dem", Function::Demodulate },
        { "send", Function::Send },
        { "neg", Function::Neg },
        { "s", Function::S },
        { "c", Function::C },
        { "b", Function::B },
        { "t", Function::True },
        { "f", Function::False },
        { "i", Function::I },
        { "cons", Function::Cons },
        { "car", Function::Car },
        { "cdr", Function::Cdr },
        { "nil", Function::Nil },
        { "isnil", Function::IsNil },
        { "vec", Function::Vec },
        { "draw", Function::Draw },
        { "chkb", Function::Checkerboard },
        { "multipledraw", Function::MultipleDraw },
        { "if0", Function::If0 },
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

bool parseTokenText(SymTable& symTable,
                    const string& text,
                    vector<Token>* pTokens,
                    string* pMsg)
{
    ParseState state = ParseState::BeforeToken;
    string str;
    vector<Token> tokens;

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
            ch != '$' &&
            ch != '{' &&
            ch != '}' &&
            ch != '(' &&
            ch != ')' &&
            ch != ',' &&
            ch != '"';

        if (state == ParseState::InSignal)
        {
            if (ch == '"')
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Signal);
                token.m_signalData.m_signal = str;
                str.clear();
                state = ParseState::AfterToken;
            }
            else if (ch == '0' || ch == '1')
            {
                str.push_back(ch);
            }
            else
            {
                if (pMsg) *pMsg = "Bad character in signal";
                return false;
            }
            continue;
        }

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
            else if (str == "=")
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Assign);
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
                uint32_t symId = symTable.getOrAdd(str);
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Symbol);
                token.m_symbolData.m_symId = symId;
            }

            str.clear();
            state = ParseState::AfterToken;
        }

        if (pos == size)
        {
            break;
        }

        if (ch == '$')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Apply);
            state = ParseState::BeforeToken;
            continue;
        }

        if (ch == '{')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::LGroup);
            state = ParseState::BeforeToken;
            continue;
        }

        if (ch == '}')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::RGroup);
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

        if (ch == ',')
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Comma);
            state = ParseState::BeforeToken;
            continue;
        }

        if (ch == '"')
        {
            state = ParseState::InSignal;
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

bool formatTokenText(const SymTable& symTable,
                     const vector<Token>& tokens,
                     string* pText,
                     string* pMsg)
{
    string text;
    bool needSpace = false;
    for (const Token& token : tokens)
    {
        TokenType tokenType = token.getTokenType();

        if (tokenType == TokenType::RGroup ||
            tokenType == TokenType::RParen ||
            tokenType == TokenType::Comma)
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
        case TokenType::Integer:
        {
            text += Int::format(token.m_integerData.m_value);
            break;
        }
        case TokenType::Variable:
        {
            text += 'x';
            text += strprintf("%" PRIu32 "", token.m_variableData.m_varId);
            break;
        }
        case TokenType::Symbol:
        {
            string symName;
            if (!symTable.getName(token.m_symbolData.m_symId, &symName))
            {
                if (pMsg) *pMsg = "Unknown symbol";
                return false;
            }
            text += symName;
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
        case TokenType::Assign:
        {
            text += "=";
            break;
        }
        case TokenType::LGroup:
        {
            text += "{";
            break;
        }
        case TokenType::RGroup:
        {
            text += "}";
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
        case TokenType::Comma:
        {
            text += ",";
            break;
        }
        default:
        {
            if (pMsg) *pMsg = "Unexpected token type";
            return false;
        }
        }

        needSpace =
            (tokenType != TokenType::LGroup &&
             tokenType != TokenType::LParen);
    }

    if (pText) *pText = text;
    return true;
}
