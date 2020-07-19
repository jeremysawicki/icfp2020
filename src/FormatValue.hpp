#ifndef FORMATVALUE_HPP
#define FORMATVALUE_HPP

#include "Common.hpp"

class Token;
class Value;
class SymTable;

bool formatValue(Value& value,
                 std::vector<Token>* pTokens,
                 std::string* pMsg = nullptr);

bool formatValueText(const SymTable& symTable,
                     Value& value,
                     std::string* pText,
                     std::string* pMsg = nullptr);

#endif
