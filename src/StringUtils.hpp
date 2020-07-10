#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include "Common.hpp"

std::string strprintf(const char* format, ...);
std::string vstrprintf(const char* format, va_list ap);

int strcmpNoCase(const char* s1, const char* s2);

#endif
