#include "ParseUtils.hpp"
#include <limits>

using std::string;

namespace
{
    template<typename T, typename F>
    bool parseIntImpl(const string& str, T* pValue, F&& f)
    {
        const char* nptr = str.c_str();
        char* endptr = nullptr;
        auto value = f(nptr, &endptr, 10);
        if (*nptr == '\0' || *endptr != '\0' || value < std::numeric_limits<T>::min() || value > std::numeric_limits<T>::max())
        {
            return false;
        }
        if (pValue)
        {
            *pValue = (T)value;
        }
        return true;
    }

    template<typename T, typename F>
    bool parseFloatImpl(const string& str, T* pValue, F&& f)
    {
        const char* nptr = str.c_str();
        char* endptr = nullptr;
        auto value = f(nptr, &endptr);
        if (*nptr == '\0' || *endptr != '\0')
        {
            return false;
        }
        if (pValue)
        {
            *pValue = (T)value;
        }
        return true;
    }
}

bool parseU32(const string& str, uint32_t* pValue)
{
    return parseIntImpl(str, pValue, strtoul);
}

bool parseI32(const string& str, int32_t* pValue)
{
    return parseIntImpl(str, pValue, strtol);
}

bool parseU64(const string& str, uint64_t* pValue)
{
    return parseIntImpl(str, pValue, strtoull);
}

bool parseI64(const string& str, int64_t* pValue)
{
    return parseIntImpl(str, pValue, strtoll);
}

bool parseFloat(const string& str, float* pValue)
{
    return parseFloatImpl(str, pValue, strtof);
}

bool parseDouble(const string& str, double* pValue)
{
    return parseFloatImpl(str, pValue, strtod);
}
