#ifndef INT_HPP
#define INT_HPP

#include "Common.hpp"
#include <limits>

#define INT_IMPL_32 0
#define INT_IMPL_64 1
#define INT_IMPL_GMP 0

#define INT_CHECK_OVERFLOW 1

#if INT_IMPL_32 || INT_IMPL_64
template<typename T>
class BasicInt
{
public:
    BasicInt() : m_value() {}

    BasicInt(int32_t value) : m_value(value) {}

    static bool inc(BasicInt& r,
                    const BasicInt& a,
                    std::string* pMsg)
    {
#if INT_CHECK_OVERFLOW
        if (a.m_value == std::numeric_limits<T>::max())
        {
            if (pMsg) *pMsg = "Increment overflow";
            return false;
        }
#endif
        r.m_value = a.m_value + 1;
        return true;
    }

    static bool dec(BasicInt& r,
                    const BasicInt& a,
                    std::string* pMsg)
    {
#if INT_CHECK_OVERFLOW
        if (a.m_value == std::numeric_limits<T>::min())
        {
            if (pMsg) *pMsg = "Decrement overflow";
            return false;
        }
#endif
        r.m_value = a.m_value - 1;
        return true;
    }

    static bool neg(BasicInt& r,
                    const BasicInt& a,
                    std::string* pMsg)
    {
#if INT_CHECK_OVERFLOW
        if (a.m_value == std::numeric_limits<T>::min())
        {
            if (pMsg) *pMsg = "Negation overflow";
            return false;
        }
#endif
        r.m_value = -a.m_value;
        return true;
    }

    static bool add(BasicInt& r,
                    const BasicInt& a,
                    const BasicInt& b,
                    std::string* pMsg)
    {
#if INT_CHECK_OVERFLOW
        if (__builtin_add_overflow(a.m_value, b.m_value, &r.m_value))
        {
            if (pMsg) *pMsg = "Addition overflow";
            return false;
        }
#else
        r.m_value = a.m_value + b.m_value;
#endif
        return true;
    }

    static bool sub(BasicInt& r,
                    const BasicInt& a,
                    const BasicInt& b,
                    std::string* pMsg)
    {
#if INT_CHECK_OVERFLOW
        if (__builtin_sub_overflow(a.m_value, b.m_value, &r.m_value))
        {
            if (pMsg) *pMsg = "Subtraction overflow";
            return false;
        }
#else
        r.m_value = a.m_value - b.m_value;
#endif
        return true;
    }

    static bool mul(BasicInt& r,
                    const BasicInt& a,
                    const BasicInt& b,
                    std::string* pMsg)
    {
#if INT_CHECK_OVERFLOW
        if (__builtin_mul_overflow(a.m_value, b.m_value, &r.m_value))
        {
            if (pMsg) *pMsg = "Multiplication overflow";
            return false;
        }
#else
        r.m_value = a.m_value * b.m_value;
#endif
        return true;
    }

    static bool div(BasicInt& r,
                    const BasicInt& a,
                    const BasicInt& b,
                    std::string* pMsg)
    {
        if (b.m_value == 0)
        {
            if (pMsg) *pMsg = "Division by zero";
            return false;
        }
        if (a.m_value == std::numeric_limits<T>::min() &&
            b.m_value == -1)
        {
            if (pMsg) *pMsg = "Division overflow";
            return false;
        }
        r.m_value = a.m_value / b.m_value;
        return true;
    }

    static bool eq(const BasicInt& a, const BasicInt& b)
    { return a.m_value == b.m_value; }
    static bool ne(const BasicInt& a, const BasicInt& b)
    { return a.m_value != b.m_value; }
    static bool lt(const BasicInt& a, const BasicInt& b)
    { return a.m_value < b.m_value; }
    static bool gt(const BasicInt& a, const BasicInt& b)
    { return a.m_value > b.m_value; }
    static bool le(const BasicInt& a, const BasicInt& b)
    { return a.m_value <= b.m_value; }
    static bool ge(const BasicInt& a, const BasicInt& b)
    { return a.m_value >= b.m_value; }

    static std::string format(const BasicInt& a)
    {
        char buf[128];
        snprintf(buf, 128, "%" PRIi64 "", (int64_t)a.m_value);
        return buf;
    }

    static bool parse(const std::string& str, bool* pInRange, BasicInt* pValue)
    {
        size_t pos = 0;
        size_t size = str.size();
        bool neg = false;
        if (pos < size && str[pos] == '-')
        {
            neg = true;
            pos++;
        }
        if (pos == size)
        {
            return false;
        }
        for (size_t tmpPos = pos; tmpPos < size; tmpPos++)
        {
            char ch = str[tmpPos];
            if (ch < '0' || ch > '9')
            {
                return false;
            }
        }
        T value = 0;
        for (; pos < size; pos++)
        {
            char ch = str[pos];
            T digit = ch - '0';
            if (neg)
            {
                if (value < std::numeric_limits<T>::min() / 10)
                {
                    *pInRange = false;
                    return true;
                }
                value *= 10;
                if (value < std::numeric_limits<T>::min() + digit)
                {
                    *pInRange = false;
                    return true;
                }
                value -= digit;
            }
            else
            {
                if (value > std::numeric_limits<T>::max() / 10)
                {
                    *pInRange = false;
                    return true;
                }
                value *= 10;
                if (value > std::numeric_limits<T>::max() - digit)
                {
                    *pInRange = false;
                    return true;
                }
                value += digit;
            }
        }
        *pInRange = true;
        pValue->m_value = value;
        return true;
    }

    static bool getValue(const BasicInt& a, int64_t* pValue, std::string* pMsg)
    {
        *pValue = a.m_value;
        return true;
    }

private:
    T m_value;
};
#endif

#if INT_IMPL_GMP
#include <gmpxx.h>
class GmpInt
{
public:
    GmpInt() : m_value() {}

    GmpInt(int32_t value) : m_value(value) {}

    static bool inc(GmpInt& r,
                    const GmpInt& a,
                    std::string* pMsg)
    {
        r.m_value = a.m_value + 1;
        return true;
    }

    static bool dec(GmpInt& r,
                    const GmpInt& a,
                    std::string* pMsg)
    {
        r.m_value = a.m_value - 1;
        return true;
    }

    static bool neg(GmpInt& r,
                    const GmpInt& a,
                    std::string* pMsg)
    {
        r.m_value = -a.m_value;
        return true;
    }

    static bool add(GmpInt& r,
                    const GmpInt& a,
                    const GmpInt& b,
                    std::string* pMsg)
    {
        r.m_value = a.m_value + b.m_value;
        return true;
    }

    static bool sub(GmpInt& r,
                    const GmpInt& a,
                    const GmpInt& b,
                    std::string* pMsg)
    {
        r.m_value = a.m_value - b.m_value;
        return true;
    }

    static bool mul(GmpInt& r,
                    const GmpInt& a,
                    const GmpInt& b,
                    std::string* pMsg)
    {
        r.m_value = a.m_value * b.m_value;
        return true;
    }

    static bool div(GmpInt& r,
                    const GmpInt& a,
                    const GmpInt& b,
                    std::string* pMsg)
    {
        if (b.m_value == 0)
        {
            if (pMsg) *pMsg = "Division by zero";
            return false;
        }
        r.m_value = a.m_value / b.m_value;
        return true;
    }

    static bool eq(const GmpInt& a, const GmpInt& b)
    { return a.m_value == b.m_value; }
    static bool ne(const GmpInt& a, const GmpInt& b)
    { return a.m_value != b.m_value; }
    static bool lt(const GmpInt& a, const GmpInt& b)
    { return a.m_value < b.m_value; }
    static bool gt(const GmpInt& a, const GmpInt& b)
    { return a.m_value > b.m_value; }
    static bool le(const GmpInt& a, const GmpInt& b)
    { return a.m_value <= b.m_value; }
    static bool ge(const GmpInt& a, const GmpInt& b)
    { return a.m_value >= b.m_value; }

    static std::string format(const GmpInt& a)
    {
        return a.m_value.get_str();
    }

    static bool parse(const std::string& str, bool* pInRange, GmpInt* pValue)
    {
        size_t pos = 0;
        size_t size = str.size();
        if (pos < size && str[pos] == '-')
        {
            pos++;
        }
        if (pos == size)
        {
            return false;
        }
        for (size_t tmpPos = pos; tmpPos < size; tmpPos++)
        {
            char ch = str[tmpPos];
            if (ch < '0' || ch > '9')
            {
                return false;
            }
        }
        if (pValue->m_value.set_str(str, 10))
        {
            // Should not happen
            *pInRange = false;
            return true;
        }
        *pInRange = true;
        return true;
    }

    static bool getValue(const GmpInt& a, int64_t* pValue, std::string* pMsg)
    {
        if (!a.m_value.fits_slong_p())
        {
            if (pMsg) *pMsg = "Integer conversion overflow";
            return false;
        }
        *pValue = (int64_t)a.m_value.get_si();
        return true;
    }

private:
    mpz_class m_value;
};
#endif

#if INT_IMPL_32
typedef BasicInt<int32_t> Int;
#endif

#if INT_IMPL_64
typedef BasicInt<int64_t> Int;
#endif

#if INT_IMPL_GMP
typedef GmpInt Int;
#endif

#endif
