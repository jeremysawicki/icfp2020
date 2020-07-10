#include "StringUtils.hpp"

using std::string;

string strprintf(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    string str = vstrprintf(format, ap);
    va_end(ap);
    return str;
}

string vstrprintf(const char* format, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);
    size_t size = vsnprintf(nullptr, 0, format, ap2);
    va_end(ap2);
    std::unique_ptr<char[]> buf(new char[size+1]);
    vsnprintf(buf.get(), size+1, format, ap);
    return buf.get();
}

int strcmpNoCase(const char* s1, const char* s2)
{
    while (true)
    {
        char ch1 = *s1;
        char ch2 = *s2;
        if (ch1 == '\0')
        {
            if (ch2 == '\0')
                return 0;
            else
                return -1;
        }
        if (ch2 == '\0')
            return 1;
        if (ch1 >= 'a' && ch1 <= 'z')
            ch1 = 'A' + (ch1 - 'a');
        if (ch2 >= 'a' && ch2 <= 'z')
            ch2 = 'A' + (ch2 - 'a');
        if (ch1 != ch2)
        {
            if (ch1 < ch2)
                return -1;
            else
                return 1;
        }
        s1++;
        s2++;
    }
}
