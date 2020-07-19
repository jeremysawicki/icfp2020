#include "ParseValue.hpp"
#include "Token.hpp"
#include "Bindings.hpp"
#include "Value.hpp"
#include "TokenText.hpp"

using std::string;
using std::vector;

#define DEBUG 0

namespace
{
    bool parseValueImpl(const vector<Token>& tokens,
                        size_t& pos,
                        const Bindings& bindings,
                        Value& value,
                        string* pMsg)
    {
#if DEBUG
        printf("> parseValueImpl\n");
#endif
        size_t size = tokens.size();
        if (pos >= size)
        {
            if (pMsg) *pMsg = "Unexpected end of input";
            return false;
        }

        auto& token = tokens[pos++];

        switch (token.m_tokenType)
        {
        case TokenType::Apply:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Apply\n", pos - 1);
#endif
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue.init();
            value->m_applyData.m_argValue.init();

            if (!parseValueImpl(tokens,
                                pos,
                                bindings,
                                value->m_applyData.m_funcValue,
                                pMsg))
            {
                return false;
            }

            if (!parseValueImpl(tokens,
                                pos,
                                bindings,
                                value->m_applyData.m_argValue,
                                pMsg))
            {
                return false;
            }

            break;
        }
        case TokenType::Integer:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Integer\n", pos - 1);
#endif
            value->setValueType(ValueType::Integer);
            value->m_integerData.m_value = token.m_integerData.m_value;
            break;
        }
        case TokenType::Symbol:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Symbol\n", pos - 1);
#endif
            uint32_t symId = token.m_symbolData.m_symId;
            if (symId >= bindings.m_values.size())
            {
                if (pMsg) *pMsg = "Symbol out of range";
                return false;
            }
            //value = bindings.m_values[symId];
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue.init(ValueType::Closure);
            value->m_applyData.m_funcValue->m_closureData.m_func = Function::I;
            value->m_applyData.m_argValue = bindings.m_values[symId];
            break;
        }
        case TokenType::Function:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Function\n", pos - 1);
#endif
            value->setValueType(ValueType::Closure);
            Function func = token.m_functionData.m_func;
            if (func == Function::Vec) func = Function::Cons;
            value->m_closureData.m_func = func;
            break;
        }
        case TokenType::Assign:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Assign\n", pos - 1);
#endif
            if (pMsg) *pMsg = "Unexpected = token";
            return false;
        }
        case TokenType::LGroup:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::LGroup\n", pos - 1);
#endif
            if (!parseValueImpl(tokens,
                                pos,
                                bindings,
                                value,
                                pMsg))
            {
                return false;
            }

            while (true)
            {
                if (pos >= size)
                {
                    if (pMsg) *pMsg = "Unexpected end of input";
                    return false;
                }

                if (tokens[pos].m_tokenType == TokenType::RGroup)
                {
                    pos++;
                    break;
                }

                Value applyValue;
                applyValue.init(ValueType::Apply);
                //applyValue->m_applyData.m_funcValue = value;
                applyValue->m_applyData.m_funcValue.init();
                *applyValue->m_applyData.m_funcValue = *value;
                applyValue->m_applyData.m_argValue.init();
                if (!parseValueImpl(tokens,
                                    pos,
                                    bindings,
                                    applyValue->m_applyData.m_argValue,
                                    pMsg))
                {
                    return false;
                }
                //value = applyValue;
                *value = *applyValue;
            }

            break;
        }
        case TokenType::RGroup:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::RGroup\n", pos - 1);
#endif
            if (pMsg) *pMsg = "Unexpected } token";
            return false;
        }
        case TokenType::LParen:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::LParen\n", pos - 1);
#endif
            Value tailValue = value;

            if (pos >= size)
            {
                if (pMsg) *pMsg = "Unexpected end of input";
                return false;
            }

            if (tokens[pos].m_tokenType == TokenType::RParen)
            {
                pos++;
            }
            else
            {
                while (true)
                {
                    tailValue->setValueType(ValueType::Closure);
                    tailValue->m_closureData.m_func = Function::Cons;
                    tailValue->m_closureData.m_size = 2;
                    tailValue->m_closureData.m_args[0].init();
                    tailValue->m_closureData.m_args[1].init();

                    if (!parseValueImpl(tokens,
                                        pos,
                                        bindings,
                                        tailValue->m_closureData.m_args[0],
                                        pMsg))
                    {
                        return false;
                    }

                    tailValue = tailValue->m_closureData.m_args[1];

                    if (pos >= size)
                    {
                        if (pMsg) *pMsg = "Unexpected end of input";
                        return false;
                    }

                    if (tokens[pos].m_tokenType == TokenType::RParen)
                    {
                        pos++;
                        break;
                    }
                    else if (tokens[pos].m_tokenType == TokenType::Comma)
                    {
                        pos++;
                    }
                    else
                    {
                        if (pMsg) *pMsg = "Expected , or )";
                        return false;
                    }
                }
            }

            tailValue->setValueType(ValueType::Closure);
            tailValue->m_closureData.m_func = Function::Nil;

            break;
        }
        case TokenType::RParen:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::RParen\n", pos - 1);
#endif
            if (pMsg) *pMsg = "Unexpected ) token";
            return false;
        }
        case TokenType::Comma:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Comma\n", pos - 1);
#endif
            if (pMsg) *pMsg = "Unexpected , token";
            return false;
        }
        case TokenType::Signal:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Signal\n", pos - 1);
#endif
            value->setValueType(ValueType::Signal);
            value->m_signalData.m_signal = token.m_signalData.m_signal;
            break;
        }
        default:
        {
            if (pMsg) *pMsg = "Unexpected token type";
            return false;
        }
        }

#if DEBUG
        printf("< parseValueImpl\n");
#endif
        return true;
    }
}

bool parseValue(const vector<Token>& tokens,
                const Bindings& bindings,
                Value& value,
                string* pMsg)
{
    size_t pos = 0;
    value.init();
    if (!parseValueImpl(tokens,
                        pos,
                        bindings,
                        value,
                        pMsg))
    {
        return false;
    }

    if (pos != tokens.size())
    {
        if (pMsg) *pMsg = "Unexpected extra input";
        return false;
    }

    return true;
}

bool parseBindings(const vector<Token>& tokens,
                   Bindings& bindings,
                   string* pMsg)
{
    size_t pos = 0;
    size_t size = tokens.size();
    while (pos < size)
    {
        auto& symToken = tokens[pos++];
        if (symToken.m_tokenType != TokenType::Symbol)
        {
            if (pMsg) *pMsg = "Expected symbol";
            return false;
        }

        uint32_t symId = symToken.m_symbolData.m_symId;

        if (symId >= bindings.m_values.size())
        {
            if (pMsg) *pMsg = "Symbol out of range";
            return false;
        }

        if (bindings.m_values[symId]->m_valueType != ValueType::Invalid)
        {
            if (pMsg) *pMsg = "Duplicate binding";
            return false;
        }

        bindings.m_order.push_back(symId);

        if (pos >= size)
        {
            if (pMsg) *pMsg = "Unexpected end of input";
            return false;
        }

        auto& assignToken = tokens[pos++];
        if (assignToken.m_tokenType != TokenType::Assign)
        {
            if (pMsg) *pMsg = "Expected =";
            return false;
        }

        if (!parseValueImpl(tokens,
                            pos,
                            bindings,
                            bindings.m_values[symId],
                            pMsg))
        {
            return false;
        }
    }

    return true;
}

bool parseValueText(SymTable& symTable,
                    const string& text,
                    const Bindings& bindings,
                    Value& value,
                    string* pMsg)
{
    vector<Token> tokens;
    if (!parseTokenText(symTable, text, &tokens, pMsg))
    {
        return false;
    }

    if (!parseValue(tokens, bindings, value, pMsg))
    {
        return false;
    }

    return true;
}
