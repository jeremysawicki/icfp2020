#include "FormatValue.hpp"
#include "Token.hpp"
#include "Value.hpp"
#include "Eval.hpp"
#include "TokenText.hpp"

using std::string;
using std::vector;

namespace
{
    bool checkIsList(Value value,
                     vector<Value>* pValues,
                     bool* pIsList,
                     string* pMsg)
    {
        vector<Value> values;
        while (true)
        {
            if (!eval(value, pMsg)) return false;
            if (value->m_valueType == ValueType::Closure &&
                value->m_closureData.m_func == Function::Nil &&
                value->m_closureData.m_size == 0)
            {
                break;
            }
            if (value->m_valueType != ValueType::Closure ||
                value->m_closureData.m_func != Function::Cons ||
                value->m_closureData.m_size != 2)
            {
                *pIsList = false;
                return true;
            }
            values.push_back(value->m_closureData.m_args[0]);
            value = value->m_closureData.m_args[1];
        }
        *pValues = std::move(values);
        *pIsList = true;
        return true;
    }

    bool formatValueImpl(Value& value,
                         vector<Token>& tokens,
                         string* pMsg)
    {
        if (!value)
        {
            if (pMsg) *pMsg = "Null value";
            return false;
        }

        if (!eval(value, pMsg))
        {
            return false;
        }

        switch (value->m_valueType)
        {
        case ValueType::Invalid:
        {
            if (pMsg) *pMsg = "Invalid value";
            return false;
        }
        case ValueType::Apply:
        {
            if (pMsg) *pMsg = "Unevaluated value";
            return false;
        }
        case ValueType::Integer:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Integer);
            token.m_integerData.m_value = value->m_integerData.m_value;
            break;
        }
        case ValueType::Closure:
        {
            vector<Value> listValues;
            bool isList = false;
            if (!checkIsList(value, &listValues, &isList, pMsg)) return false;
            if (isList)
            {
                {
                    auto& token = tokens.emplace_back();
                    token.setTokenType(TokenType::LParen);
                }

                for (size_t i = 0; i < listValues.size(); i++)
                {
                    if (i != 0)
                    {
                        auto& token = tokens.emplace_back();
                        token.setTokenType(TokenType::Comma);
                    }

                    if (!formatValueImpl(listValues[i], tokens, pMsg))
                    {
                        return false;
                    }
                }

                {
                    auto& token = tokens.emplace_back();
                    token.setTokenType(TokenType::RParen);
                }

                break;
            }

            Function func = value->m_closureData.m_func;
            uint32_t size = value->m_closureData.m_size;
            auto& args = value->m_closureData.m_args;

            for (uint32_t iArg = 0; iArg < size; iArg++)
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Apply);
            }

            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Function);
                token.m_functionData.m_func = func;
            }

            for (uint32_t iArg = 0; iArg < size; iArg++)
            {
                if (!formatValueImpl(args[iArg], tokens, pMsg))
                {
                    return false;
                }
            }

            break;
        }
        case ValueType::Signal:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Signal);
            token.m_signalData.m_signal = value->m_signalData.m_signal;
            break;
        }
        case ValueType::Picture:
        {
            if (pMsg) *pMsg = "Picture value";
            return false;
        }
        default:
        {
            if (pMsg) *pMsg = "Unknown value type";
            return false;
        }
        }

        return true;
    }
}

bool formatValue(Value& value,
                 vector<Token>* pTokens,
                 string* pMsg)
{
    vector<Token> tokens;
    if (!formatValueImpl(value, tokens, pMsg)) return false;
    if (pTokens) *pTokens = std::move(tokens);
    return true;
}

bool formatValueText(const SymTable& symTable,
                     Value& value,
                     string* pText,
                     string* pMsg)
{
    vector<Token> tokens;
    if (!formatValue(value, &tokens, pMsg))
    {
        return false;
    }

    string text;
    if (!formatTokenText(symTable, tokens, &text, pMsg))
    {
        return false;
    }

    if (pText) *pText = std::move(text);
    return true;
}
