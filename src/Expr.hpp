#ifndef EXPR_HPP
#define EXPR_HPP

#include "Common.hpp"
#include "Int.hpp"
#include "Function.hpp"

enum class ExprType : uint32_t
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
    Builtin,
    Let,
    Paren
};

class Expr;

class ExprApplyData
{
public:
    std::unique_ptr<Expr> m_funcExpr;
    std::unique_ptr<Expr> m_argExpr;
};

class ExprLambdaData
{
public:
    uint32_t m_varId = 0;
    std::unique_ptr<Expr> m_bodyExpr;
};

class ExprIfData
{
public:
    std::unique_ptr<Expr> m_condExpr;
    std::unique_ptr<Expr> m_trueExpr;
    std::unique_ptr<Expr> m_falseExpr;
};

class ExprAndData
{
public:
    std::unique_ptr<Expr> m_exprA;
    std::unique_ptr<Expr> m_exprB;
};

class ExprOrData
{
public:
    std::unique_ptr<Expr> m_exprA;
    std::unique_ptr<Expr> m_exprB;
};

class ExprIntegerData
{
public:
    Int m_value;
};

class ExprBooleanData
{
public:
    bool m_value = false;
};

class ExprVariableData
{
public:
    uint32_t m_varId = 0;
};

class ExprFunctionData
{
public:
    Function m_func = Function::Invalid;
};

class ExprBuiltinData
{
public:
    Function m_func = Function::Invalid;
    std::vector<std::unique_ptr<Expr>> m_argExprs;
};

class ExprLetData
{
public:
    std::vector<uint32_t> m_varIds;
    std::unique_ptr<Expr> m_valExpr;
    std::unique_ptr<Expr> m_bodyExpr;
};

class ExprParenData
{
public:
    std::vector<std::unique_ptr<Expr>> m_subExprs;
};

class Expr
{
public:
    Expr() : m_exprType(ExprType::Invalid) {}

    Expr(const Expr& other) = delete;

    Expr(Expr&& other)
    {
        moveFrom(std::move(other));
    }

    ~Expr()
    {
        destroy();
    }

    Expr& operator=(const Expr& other) = delete;

    Expr& operator=(Expr&& other)
    {
        if (this != &other)
        {
            destroy();
            moveFrom(std::move(other));
        }
        return *this;
    }

    ExprType getExprType() const { return m_exprType; }

    void setExprType(ExprType exprType)
    {
        if (exprType != m_exprType)
        {
            destroy();
            m_exprType = exprType;
            switch (exprType)
            {
            case ExprType::Invalid: break;
            case ExprType::Apply: new(&m_applyData) ExprApplyData; break;
            case ExprType::Lambda: new(&m_lambdaData) ExprLambdaData; break;
            case ExprType::If: new(&m_ifData) ExprIfData; break;
            case ExprType::And: new(&m_andData) ExprAndData; break;
            case ExprType::Or: new(&m_orData) ExprOrData; break;
            case ExprType::Integer: new(&m_integerData) ExprIntegerData; break;
            case ExprType::Boolean: new(&m_booleanData) ExprBooleanData; break;
            case ExprType::Variable: new(&m_variableData) ExprVariableData; break;
            case ExprType::Function: new(&m_functionData) ExprFunctionData; break;
            case ExprType::Builtin: new(&m_builtinData) ExprBuiltinData; break;
            case ExprType::Let: new(&m_letData) ExprLetData; break;
            case ExprType::Paren: new(&m_parenData) ExprParenData; break;
            }
        }
    }

private:
    void destroy()
    {
        switch (m_exprType)
        {
        case ExprType::Invalid: break;
        case ExprType::Apply: m_applyData.~ExprApplyData(); break;
        case ExprType::Lambda: m_lambdaData.~ExprLambdaData(); break;
        case ExprType::If: m_ifData.~ExprIfData(); break;
        case ExprType::And: m_andData.~ExprAndData(); break;
        case ExprType::Or: m_orData.~ExprOrData(); break;
        case ExprType::Integer: m_integerData.~ExprIntegerData(); break;
        case ExprType::Boolean: m_booleanData.~ExprBooleanData(); break;
        case ExprType::Variable: m_variableData.~ExprVariableData(); break;
        case ExprType::Function: m_functionData.~ExprFunctionData(); break;
        case ExprType::Builtin: m_builtinData.~ExprBuiltinData(); break;
        case ExprType::Let: m_letData.~ExprLetData(); break;
        case ExprType::Paren: m_parenData.~ExprParenData(); break;
        }
    }
    void moveFrom(Expr&& other)
    {
        m_exprType = other.m_exprType;

        switch (other.m_exprType)
        {
        case ExprType::Invalid: break;
        case ExprType::Apply: new(&m_applyData) ExprApplyData(std::move(other.m_applyData)); break;
        case ExprType::Lambda: new(&m_lambdaData) ExprLambdaData(std::move(other.m_lambdaData)); break;
        case ExprType::If: new(&m_ifData) ExprIfData(std::move(other.m_ifData)); break;
        case ExprType::And: new(&m_andData) ExprAndData(std::move(other.m_andData)); break;
        case ExprType::Or: new(&m_orData) ExprOrData(std::move(other.m_orData)); break;
        case ExprType::Integer: new(&m_integerData) ExprIntegerData(std::move(other.m_integerData)); break;
        case ExprType::Boolean: new(&m_booleanData) ExprBooleanData(std::move(other.m_booleanData)); break;
        case ExprType::Variable: new(&m_variableData) ExprVariableData(std::move(other.m_variableData)); break;
        case ExprType::Function: new(&m_functionData) ExprFunctionData(std::move(other.m_functionData)); break;
        case ExprType::Builtin: new(&m_builtinData) ExprBuiltinData(std::move(other.m_builtinData)); break;
        case ExprType::Let: new(&m_letData) ExprLetData(std::move(other.m_letData)); break;
        case ExprType::Paren: new(&m_parenData) ExprParenData(std::move(other.m_parenData)); break;
        }
    }

public:
    ExprType m_exprType;
    union
    {
        ExprApplyData m_applyData;
        ExprLambdaData m_lambdaData;
        ExprIfData m_ifData;
        ExprAndData m_andData;
        ExprOrData m_orData;
        ExprIntegerData m_integerData;
        ExprBooleanData m_booleanData;
        ExprVariableData m_variableData;
        ExprFunctionData m_functionData;
        ExprBuiltinData m_builtinData;
        ExprLetData m_letData;
        ExprParenData m_parenData;
    };
};

#endif
