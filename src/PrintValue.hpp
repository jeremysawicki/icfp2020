#ifndef PRINTVALUE_HPP
#define PRINTVALUE_HPP

#include "Common.hpp"

class Value;

bool printValue(std::string* pStr,
                Value& value,
                bool force,
                std::string* pMsg = nullptr);

bool printValue(Value& value,
                bool force,
                std::string* pMsg = nullptr);

#endif
