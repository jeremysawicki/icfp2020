#include "Modem.hpp"
#include "Value.hpp"
#include "Eval.hpp"

using std::string;

bool modulate(Value& value,
              string& signal,
              string* pMsg)
{
    if (!eval(value, pMsg)) return false;
    if (value->m_valueType == ValueType::Integer)
    {
        size_t pos = signal.size();
        Int a = value->m_integerData.m_value;
        bool neg = Int::lt(a, Int(0));
        uint32_t digitCount = 0;
        while (!Int::eq(a, Int(0)))
        {
            digitCount++;
            Int base(16);
            Int q;
            if (!Int::div(q, a, base, pMsg)) return false;
            Int r;
            if (!Int::mul(r, q, base, pMsg)) return false;
            if (!Int::sub(r, a, r, pMsg)) return false;
            if (neg)
            {
                if (!Int::neg(r, r, pMsg)) return false;
            }
            char c3 = '0';
            if (Int::ge(r, Int(8)))
            {
                c3 = '1';
                if (!Int::sub(r, r, Int(8), pMsg)) return false;
            }
            char c2 = '0';
            if (Int::ge(r, Int(4)))
            {
                c2 = '1';
                if (!Int::sub(r, r, Int(4), pMsg)) return false;
            }
            char c1 = '0';
            if (Int::ge(r, Int(2)))
            {
                c1 = '1';
                if (!Int::sub(r, r, Int(2), pMsg)) return false;
            }
            char c0 = '0';
            if (Int::ge(r, Int(1)))
            {
                c0 = '1';
                if (!Int::sub(r, r, Int(1), pMsg)) return false;
            }
            signal += c0;
            signal += c1;
            signal += c2;
            signal += c3;
            a = q;
        }
        signal += '0';
        for (uint32_t i = 0; i < digitCount; i++)
        {
            signal += '1';
        }
        if (neg)
        {
            signal += '0';
            signal += '1';
        }
        else
        {
            signal += '1';
            signal += '0';
        }
        std::reverse(signal.begin() + pos, signal.end());
        return true;
    }
    if (value->m_valueType == ValueType::Closure &&
        value->m_closureData.m_func == Function::Nil &&
        value->m_closureData.m_size == 0)
    {
        signal += '0';
        signal += '0';
        return true;
    }
    if (value->m_valueType == ValueType::Closure &&
        value->m_closureData.m_func == Function::Cons &&
        value->m_closureData.m_size == 2)
    {
        signal += '1';
        signal += '1';
        if (!modulate(value->m_closureData.m_args[0], signal, pMsg)) return false;
        if (!modulate(value->m_closureData.m_args[1], signal, pMsg)) return false;
        return true;
    }

    if (pMsg) *pMsg = "Bad argument type";
    return false;
}

namespace
{
    bool demodulateImpl(const string& signal,
                        size_t& pos,
                        Value& value,
                        string* pMsg)
    {
        size_t size = signal.size();
        if (size - pos < 2)
        {
            if (pMsg) *pMsg = "Bad signal";
            return false;
        }
        char b0 = signal[pos++];
        char b1 = signal[pos++];
        if (b0 == '0' && b1 == '0')
        {
            value->setValueType(ValueType::Closure);
            value->m_closureData.m_func = Function::Nil;
            return true;
        }
        if (b0 == '1' && b1 == '1')
        {
            value->setValueType(ValueType::Closure);
            value->m_closureData.m_func = Function::Cons;
            value->m_closureData.m_size = 2;
            value->m_closureData.m_args[0].init();
            value->m_closureData.m_args[1].init();
            if (!demodulateImpl(signal, pos, value->m_closureData.m_args[0], pMsg)) return false;
            if (!demodulateImpl(signal, pos, value->m_closureData.m_args[1], pMsg)) return false;
            return true;
        }
        bool neg = false;
        if (b0 == '0' && b1 == '1')
        {
            neg = false;
        }
        else if (b0 == '1' && b1 == '0')
        {
            neg = true;
        }
        else
        {
            if (pMsg) *pMsg = "Bad signal";
            return false;
        }
        uint32_t digitCount = 0;
        while (true)
        {
            if (pos == size)
            {
                if (pMsg) *pMsg = "Bad signal";
                return false;
            }
            char b = signal[pos++];
            if (b == '0') break;
            if (b != '1')
            {
                if (pMsg) *pMsg = "Bad signal";
                return false;
            }
            digitCount++;
        }
        if (neg && digitCount == 0)
        {
            if (pMsg) *pMsg = "Bad signal";
            return false;
        }
        if (size - pos < digitCount * 4)
        {
            if (pMsg) *pMsg = "Bad signal";
            return false;
        }
        Int a(0);
        Int one(neg ? -1 : 1);
        Int two(2);
        for (uint32_t i = 0; i < digitCount * 4; i++)
        {
            char b = signal[pos++];
            if (!Int::mul(a, a, two, pMsg))
            {
                return false;
            }
            if (b == '0')
            {
            }
            else if (b == '1')
            {
                if (!Int::add(a, a, one, pMsg))
                {
                    return false;
                }
            }
            else
            {
                if (pMsg) *pMsg = "Bad signal";
                return false;
            }
        }
        value->setValueType(ValueType::Integer);
        value->m_integerData.m_value = std::move(a);
        return true;
    }
}

bool demodulate(const string& signal,
                Value& value,
                string* pMsg)
{
    size_t pos = 0;
    return demodulateImpl(signal, pos, value, pMsg);
}
