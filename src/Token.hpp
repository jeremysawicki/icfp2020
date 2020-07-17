#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "Common.hpp"
#include "Int.hpp"
#include "Function.hpp"

enum class TokenType : uint32_t
{
    Invalid,
    Apply,
    Lambda,
    If,
    And,
    Or,
    Integer,
    Boolean,
    Variable,
    Function,
    Let,
    Assign,
    LParen,
    RParen
};

class Token;

class TokenIntegerData
{
public:
    Int m_value;
};

class TokenBooleanData
{
public:
    bool m_value = false;
};

class TokenVariableData
{
public:
    uint32_t m_varId = 0;
};

class TokenFunctionData
{
public:
    Function m_func = Function::Invalid;
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
            case TokenType::Boolean: new(&m_booleanData) TokenBooleanData; break;
            case TokenType::Variable: new(&m_variableData) TokenVariableData; break;
            case TokenType::Function: new(&m_functionData) TokenFunctionData; break;
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
        case TokenType::Boolean: m_booleanData.~TokenBooleanData(); break;
        case TokenType::Variable: m_variableData.~TokenVariableData(); break;
        case TokenType::Function: m_functionData.~TokenFunctionData(); break;
        default: ;
        }
    }
    void copyFrom(const Token& other)
    {
        m_tokenType = other.m_tokenType;

        switch (other.m_tokenType)
        {
        case TokenType::Integer: new(&m_integerData) TokenIntegerData(other.m_integerData); break;
        case TokenType::Boolean: new(&m_booleanData) TokenBooleanData(other.m_booleanData); break;
        case TokenType::Variable: new(&m_variableData) TokenVariableData(other.m_variableData); break;
        case TokenType::Function: new(&m_functionData) TokenFunctionData(other.m_functionData); break;
        default: ;
        }
    }
    void moveFrom(Token&& other)
    {
        m_tokenType = other.m_tokenType;

        switch (other.m_tokenType)
        {
        case TokenType::Integer: new(&m_integerData) TokenIntegerData(std::move(other.m_integerData)); break;
        case TokenType::Boolean: new(&m_booleanData) TokenBooleanData(std::move(other.m_booleanData)); break;
        case TokenType::Variable: new(&m_variableData) TokenVariableData(std::move(other.m_variableData)); break;
        case TokenType::Function: new(&m_functionData) TokenFunctionData(std::move(other.m_functionData)); break;
        default: ;
        }
    }

public:
    TokenType m_tokenType;
    union
    {
        TokenIntegerData m_integerData;
        TokenBooleanData m_booleanData;
        TokenVariableData m_variableData;
        TokenFunctionData m_functionData;
    };
};

#endif
