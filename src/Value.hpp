#ifndef VALUE_HPP
#define VALUE_HPP

#include "Common.hpp"
#include "Int.hpp"

class Closure;

enum class ValueType : uint32_t
{
    Invalid,
    Integer,
    Boolean,
    Closure
};

class Value;

class ValueIntegerData
{
public:
    Int m_value;
};

class ValueBooleanData
{
public:
    bool m_value = false;
};

class ValueClosureData
{
public:
    ValueClosureData();
    ValueClosureData(const ValueClosureData& other);
    ValueClosureData(ValueClosureData&& other);
    ~ValueClosureData();
    ValueClosureData& operator=(const ValueClosureData& other);
    ValueClosureData& operator=(ValueClosureData&& other);

    Closure* m_pClosure;
};

class Value
{
public:
    Value() : m_valueType(ValueType::Invalid) {}

    Value(const Value& other)
    {
        copyFrom(other);
    }

    Value(Value&& other)
    {
        moveFrom(std::move(other));
    }

    ~Value()
    {
        destroy();
    }

    Value& operator=(const Value& other)
    {
        if (this != &other)
        {
            destroy();
            copyFrom(other);
        }
        return *this;
    }

    Value& operator=(Value&& other)
    {
        if (this != &other)
        {
            destroy();
            moveFrom(std::move(other));
        }
        return *this;
    }

    ValueType getValueType() const { return m_valueType; }

    void setValueType(ValueType valueType)
    {
        if (valueType != m_valueType)
        {
            destroy();
            m_valueType = valueType;
            switch (valueType)
            {
            case ValueType::Invalid: break;
            case ValueType::Integer: new(&m_integerData) ValueIntegerData; break;
            case ValueType::Boolean: new(&m_booleanData) ValueBooleanData; break;
            case ValueType::Closure: new(&m_closureData) ValueClosureData; break;
            }
        }
    }

private:
    void destroy()
    {
        switch (m_valueType)
        {
        case ValueType::Invalid: break;
        case ValueType::Integer: m_integerData.~ValueIntegerData(); break;
        case ValueType::Boolean: m_booleanData.~ValueBooleanData(); break;
        case ValueType::Closure: m_closureData.~ValueClosureData(); break;
        }
    }
    void copyFrom(const Value& other)
    {
        m_valueType = other.m_valueType;

        switch (other.m_valueType)
        {
        case ValueType::Invalid: break;
        case ValueType::Integer: new(&m_integerData) ValueIntegerData(other.m_integerData); break;
        case ValueType::Boolean: new(&m_booleanData) ValueBooleanData(other.m_booleanData); break;
        case ValueType::Closure: new(&m_closureData) ValueClosureData(other.m_closureData); break;
        }
    }
    void moveFrom(Value&& other)
    {
        m_valueType = other.m_valueType;

        switch (other.m_valueType)
        {
        case ValueType::Invalid: break;
        case ValueType::Integer: new(&m_integerData) ValueIntegerData(std::move(other.m_integerData)); break;
        case ValueType::Boolean: new(&m_booleanData) ValueBooleanData(std::move(other.m_booleanData)); break;
        case ValueType::Closure: new(&m_closureData) ValueClosureData(std::move(other.m_closureData)); break;
        }
    }

public:
    ValueType m_valueType;
    union
    {
        ValueIntegerData m_integerData;
        ValueBooleanData m_booleanData;
        ValueClosureData m_closureData;
    };
};

#include "Closure.hpp"

inline ValueClosureData::ValueClosureData() :
    m_pClosure(nullptr)
{
}

inline ValueClosureData::ValueClosureData(const ValueClosureData& other) :
    m_pClosure(other.m_pClosure)
{
    if (m_pClosure) m_pClosure->addRef();
}

inline ValueClosureData::ValueClosureData(ValueClosureData&& other) :
    m_pClosure(other.m_pClosure)
{
    other.m_pClosure = nullptr;
}

inline ValueClosureData::~ValueClosureData()
{
    if (m_pClosure) m_pClosure->release();
}

inline ValueClosureData& ValueClosureData::operator=(const ValueClosureData& other)
{
    if (this != &other)
    {
        if (other.m_pClosure) other.m_pClosure->addRef();
        if (m_pClosure) m_pClosure->release();
        m_pClosure = other.m_pClosure;
    }
    return *this;
}

inline ValueClosureData& ValueClosureData::operator=(ValueClosureData&& other)
{
    if (this != &other)
    {
        m_pClosure = other.m_pClosure;
        other.m_pClosure = nullptr;
    }
    return *this;
}

#endif
