#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "Common.hpp"
#include "Int.hpp"
#include "Function.hpp"

enum class TokenType : uint32_t
{
    Invalid,
    Apply, // #17
    Integer,
    Variable,
    Symbol,
    Function,
    Assign,
    LGroup,
    RGroup,
    LParen, // #30
    RParen, // #30
    Comma, // #30
    Signal
};

class Token;

class TokenIntegerData
{
public:
    Int m_value;
};

class TokenVariableData
{
public:
    uint32_t m_varId = 0;
};

class TokenSymbolData
{
public:
    uint32_t m_symId = 0;
};

class TokenFunctionData
{
public:
    Function m_func = Function::Invalid;
};

class TokenSignalData
{
public:
    std::string m_signal;
};

class Token
{
public:
    Token() : m_tokenType(TokenType::Invalid) {}

    Token(TokenType tokenType) : m_tokenType(TokenType::Invalid)
    {
        setTokenType(tokenType);
    }

    Token(const Token& other)
    {
        copyFrom(other);
    }

    Token(Token&& other)
    {
        moveFrom(std::move(other));
    }

    ~Token()
    {
        destroy();
    }

    Token& operator=(const Token& other)
    {
        if (this != &other)
        {
            destroy();
            copyFrom(other);
        }
        return *this;
    }

    Token& operator=(Token&& other)
    {
        if (this != &other)
        {
            destroy();
            moveFrom(std::move(other));
        }
        return *this;
    }

    TokenType getTokenType() const { return m_tokenType; }

    void setTokenType(TokenType tokenType)
    {
        if (tokenType != m_tokenType)
        {
            destroy();
            m_tokenType = tokenType;
            switch (tokenType)
            {
            case TokenType::Integer: new(&m_integerData) TokenIntegerData; break;
            case TokenType::Variable: new(&m_variableData) TokenVariableData; break;
            case TokenType::Symbol: new(&m_symbolData) TokenSymbolData; break;
            case TokenType::Function: new(&m_functionData) TokenFunctionData; break;
            case TokenType::Signal: new(&m_signalData) TokenSignalData; break;
            default: ;
            }
        }
    }

private:
    void destroy()
    {
        switch (m_tokenType)
        {
        case TokenType::Integer: m_integerData.~TokenIntegerData(); break;
        case TokenType::Variable: m_variableData.~TokenVariableData(); break;
        case TokenType::Symbol: m_symbolData.~TokenSymbolData(); break;
        case TokenType::Function: m_functionData.~TokenFunctionData(); break;
        case TokenType::Signal: m_signalData.~TokenSignalData(); break;
        default: ;
        }
    }
    void copyFrom(const Token& other)
    {
        m_tokenType = other.m_tokenType;

        switch (other.m_tokenType)
        {
        case TokenType::Integer: new(&m_integerData) TokenIntegerData(other.m_integerData); break;
        case TokenType::Variable: new(&m_variableData) TokenVariableData(other.m_variableData); break;
        case TokenType::Symbol: new(&m_symbolData) TokenSymbolData(other.m_symbolData); break;
        case TokenType::Function: new(&m_functionData) TokenFunctionData(other.m_functionData); break;
        case TokenType::Signal: new(&m_signalData) TokenSignalData(other.m_signalData); break;
        default: ;
        }
    }
    void moveFrom(Token&& other)
    {
        m_tokenType = other.m_tokenType;

        switch (other.m_tokenType)
        {
        case TokenType::Integer: new(&m_integerData) TokenIntegerData(std::move(other.m_integerData)); break;
        case TokenType::Variable: new(&m_variableData) TokenVariableData(std::move(other.m_variableData)); break;
        case TokenType::Symbol: new(&m_symbolData) TokenSymbolData(std::move(other.m_symbolData)); break;
        case TokenType::Function: new(&m_functionData) TokenFunctionData(std::move(other.m_functionData)); break;
        case TokenType::Signal: new(&m_signalData) TokenSignalData(std::move(other.m_signalData)); break;
        default: ;
        }
    }

public:
    TokenType m_tokenType;
    union
    {
        TokenIntegerData m_integerData;
        TokenVariableData m_variableData;
        TokenSymbolData m_symbolData;
        TokenFunctionData m_functionData;
        TokenSignalData m_signalData;
    };
};

#endif
