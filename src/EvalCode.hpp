#ifndef EVALCODE_HPP
#define EVALCODE_HPP

#include "Common.hpp"

class Code;
class Value;

bool evalCode(const Code& code,
              Value* pValue,
              std::string* pMsg = nullptr);

#endif
