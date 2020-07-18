#ifndef VALUE_HPP
#define VALUE_HPP

#include "Common.hpp"
#include "Int.hpp"
#include "Function.hpp"
#include "Grid.hpp"
#include "Heap.hpp"

enum class ValueType : uint32_t
{
    Invalid,
    Apply,
    Integer,
    Closure,
    Signal,
    Picture
};

class Expr;
class ValueData;

class Value
{
public:
    Value();
    Value(const Value& other);
    Value(Value&& other);
    ~Value();
    Value& operator=(const Value& other);
    Value& operator=(Value&& other);
    void swap(Value& other);

    explicit operator bool() const;
    ValueData& operator*() const;
    ValueData* operator->() const;

    void init();
    void init(ValueType valueType);

private:
    ValueData* m_pData;
};

class ValueApplyData
{
public:
    Value m_funcValue;
    Value m_argValue;
};

class ValueIntegerData
{
public:
    Int m_value;
};

class ValueClosureData
{
public:
    Function m_func = Function::Invalid;
    uint32_t m_size = 0;
    Value m_args[2];
};

class ValueSignalData
{
public:
    std::string m_signal;
};

class ValuePictureData
{
public:
    Grid<uint8_t> m_picture;
};

class ValueData
{
public:
    ValueData() :
        m_valueType(ValueType::Invalid),
        m_refCount(1)
    {
    }

    ValueData(const ValueData& other)
    {
        copyFrom(other);
    }

    ValueData(ValueData&& other)
    {
        moveFrom(std::move(other));
    }

    ~ValueData()
    {
        destroy();
    }

    ValueData& operator=(const ValueData& other)
    {
        if (this != &other)
        {
            destroy();
            copyFrom(other);
        }
        return *this;
    }

    ValueData& operator=(ValueData&& other)
    {
        if (this != &other)
        {
            destroy();
            moveFrom(std::move(other));
        }
        return *this;
    }

    static ValueData* create()
    {
        ValueData* pData = (ValueData*)heap.malloc(sizeof(ValueData));
        new(pData) ValueData;
        return pData;
    }

    void addRef()
    {
        m_refCount++;
    }

    void release()
    {
        if (--m_refCount == 0)
        {
            this->~ValueData();
            heap.free(this);
        }
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
            case ValueType::Apply: new(&m_applyData) ValueApplyData; break;
            case ValueType::Integer: new(&m_integerData) ValueIntegerData; break;
            case ValueType::Closure: new(&m_closureData) ValueClosureData; break;
            case ValueType::Signal: new(&m_signalData) ValueSignalData; break;
            case ValueType::Picture: new(&m_pictureData) ValuePictureData; break;
            }
        }
    }

private:
    void destroy()
    {
        switch (m_valueType)
        {
        case ValueType::Invalid: break;
        case ValueType::Apply: m_applyData.~ValueApplyData(); break;
        case ValueType::Integer: m_integerData.~ValueIntegerData(); break;
        case ValueType::Closure: m_closureData.~ValueClosureData(); break;
        case ValueType::Signal: m_signalData.~ValueSignalData(); break;
        case ValueType::Picture: m_pictureData.~ValuePictureData(); break;
        }
    }
    void copyFrom(const ValueData& other)
    {
        m_valueType = other.m_valueType;

        switch (other.m_valueType)
        {
        case ValueType::Invalid: break;
        case ValueType::Apply: new(&m_applyData) ValueApplyData(other.m_applyData); break;
        case ValueType::Integer: new(&m_integerData) ValueIntegerData(other.m_integerData); break;
        case ValueType::Closure: new(&m_closureData) ValueClosureData(other.m_closureData); break;
        case ValueType::Signal: new(&m_signalData) ValueSignalData(other.m_signalData); break;
        case ValueType::Picture: new(&m_pictureData) ValuePictureData(other.m_pictureData); break;
        }
    }
    void moveFrom(ValueData&& other)
    {
        m_valueType = other.m_valueType;

        switch (other.m_valueType)
        {
        case ValueType::Invalid: break;
        case ValueType::Apply: new(&m_applyData) ValueApplyData(std::move(other.m_applyData)); break;
        case ValueType::Integer: new(&m_integerData) ValueIntegerData(std::move(other.m_integerData)); break;
        case ValueType::Closure: new(&m_closureData) ValueClosureData(std::move(other.m_closureData)); break;
        case ValueType::Signal: new(&m_signalData) ValueSignalData(std::move(other.m_signalData)); break;
        case ValueType::Picture: new(&m_pictureData) ValuePictureData(std::move(other.m_pictureData)); break;
        }
    }

public:
    ValueType m_valueType;
    uint32_t m_refCount;
    union
    {
        ValueApplyData m_applyData;
        ValueIntegerData m_integerData;
        ValueClosureData m_closureData;
        ValueSignalData m_signalData;
        ValuePictureData m_pictureData;
    };
};

inline Value::Value() :
    m_pData(nullptr)
{
}

inline Value::Value(const Value& other) :
    m_pData(other.m_pData)
{
    if (m_pData) m_pData->addRef();
}

inline Value::Value(Value&& other) :
    m_pData(other.m_pData)
{
    other.m_pData = nullptr;
}

inline Value::~Value()
{
    if (m_pData) m_pData->release();
}

inline Value& Value::operator=(const Value& other)
{
    if (this != &other)
    {
        if (other.m_pData) other.m_pData->addRef();
        if (m_pData) m_pData->release();
        m_pData = other.m_pData;
    }
    return *this;
}

inline Value& Value::operator=(Value&& other)
{
    if (this != &other)
    {
        if (m_pData) m_pData->release();
        m_pData = other.m_pData;
        other.m_pData = nullptr;
    }
    return *this;
}

inline void Value::swap(Value& other)
{
    using std::swap;
    swap(m_pData, other.m_pData);
}

inline Value::operator bool() const
{
    return m_pData != nullptr;
}

inline ValueData& Value::operator*() const
{
    return *m_pData;
}

inline ValueData* Value::operator->() const
{
    return m_pData;
}

inline void Value::init()
{
    if (m_pData) m_pData->release();
    m_pData = ValueData::create();
}

inline void Value::init(ValueType valueType)
{
    init();
    m_pData->setValueType(valueType);
}

inline void swap(Value& x, Value& y)
{
    x.swap(y);
}

#endif
