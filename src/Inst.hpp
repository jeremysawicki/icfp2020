#ifndef INST_HPP
#define INST_HPP

#include "Common.hpp"
#include "Int.hpp"
#include "Function.hpp"

enum class InstType : uint32_t
{
    Invalid,
    Call,
    TailCall,
    Lambda,
    Jf,
    Jmp,
    Integer,
    Boolean,
    Variable,
    Return,
    Inc,
    Dec,
    Neg,
    Not,
    Add,
    Sub,
    Mul,
    Div,
    Eq,
    Ne,
    Lt,
    Gt,
    Le,
    Ge
};

class Inst;

class InstLambdaData
{
public:
    uint32_t m_iEntry = 0;
};

class InstJfData
{
public:
    uint32_t m_iInst = 0;
};

class InstJmpData
{
public:
    uint32_t m_iInst = 0;
};

class InstIntegerData
{
public:
    Int m_value;
};

class InstBooleanData
{
public:
    bool m_value = false;
};

class InstVariableData
{
public:
    uint32_t m_slot = 0;
};

class Inst
{
public:
    Inst() : m_instType(InstType::Invalid) {}

    Inst(const Inst& other)
    {
        copyFrom(other);
    }

    Inst(Inst&& other)
    {
        moveFrom(std::move(other));
    }

    ~Inst()
    {
        destroy();
    }

    Inst& operator=(const Inst& other)
    {
        if (this != &other)
        {
            destroy();
            copyFrom(other);
        }
        return *this;
    }

    Inst& operator=(Inst&& other)
    {
        if (this != &other)
        {
            destroy();
            moveFrom(std::move(other));
        }
        return *this;
    }

    InstType getInstType() const { return m_instType; }

    void setInstType(InstType instType)
    {
        if (instType != m_instType)
        {
            destroy();
            m_instType = instType;
            switch (instType)
            {
            case InstType::Lambda: new(&m_lambdaData) InstLambdaData; break;
            case InstType::Jf: new(&m_jfData) InstJfData; break;
            case InstType::Jmp: new(&m_jmpData) InstJmpData; break;
            case InstType::Integer: new(&m_integerData) InstIntegerData; break;
            case InstType::Boolean: new(&m_booleanData) InstBooleanData; break;
            case InstType::Variable: new(&m_variableData) InstVariableData; break;
            default: ;
            }
        }
    }

private:
    void destroy()
    {
        switch (m_instType)
        {
        case InstType::Lambda: m_lambdaData.~InstLambdaData(); break;
        case InstType::Jf: m_jfData.~InstJfData(); break;
        case InstType::Jmp: m_jmpData.~InstJmpData(); break;
        case InstType::Integer: m_integerData.~InstIntegerData(); break;
        case InstType::Boolean: m_booleanData.~InstBooleanData(); break;
        case InstType::Variable: m_variableData.~InstVariableData(); break;
        default: ;
        }
    }
    void copyFrom(const Inst& other)
    {
        m_instType = other.m_instType;

        switch (other.m_instType)
        {
        case InstType::Lambda: new(&m_lambdaData) InstLambdaData(other.m_lambdaData); break;
        case InstType::Jf: new(&m_jfData) InstJfData(other.m_jfData); break;
        case InstType::Jmp: new(&m_jmpData) InstJmpData(other.m_jmpData); break;
        case InstType::Integer: new(&m_integerData) InstIntegerData(other.m_integerData); break;
        case InstType::Boolean: new(&m_booleanData) InstBooleanData(other.m_booleanData); break;
        case InstType::Variable: new(&m_variableData) InstVariableData(other.m_variableData); break;
        default: ;
        }
    }
    void moveFrom(Inst&& other)
    {
        m_instType = other.m_instType;

        switch (other.m_instType)
        {
        case InstType::Lambda: new(&m_lambdaData) InstLambdaData(std::move(other.m_lambdaData)); break;
        case InstType::Jf: new(&m_jfData) InstJfData(std::move(other.m_jfData)); break;
        case InstType::Jmp: new(&m_jmpData) InstJmpData(std::move(other.m_jmpData)); break;
        case InstType::Integer: new(&m_integerData) InstIntegerData(std::move(other.m_integerData)); break;
        case InstType::Boolean: new(&m_booleanData) InstBooleanData(std::move(other.m_booleanData)); break;
        case InstType::Variable: new(&m_variableData) InstVariableData(std::move(other.m_variableData)); break;
        default: ;
        }
    }

public:
    InstType m_instType;
    union
    {
        InstLambdaData m_lambdaData;
        InstJfData m_jfData;
        InstJmpData m_jmpData;
        InstIntegerData m_integerData;
        InstBooleanData m_booleanData;
        InstVariableData m_variableData;
    };
};

#endif
