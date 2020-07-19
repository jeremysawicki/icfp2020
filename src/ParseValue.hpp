#ifndef PARSEVALUE_HPP
#define PARSEVALUE_HPP

#include "Common.hpp"

class Token;
class Bindings;
class Value;
class SymTable;

bool parseValue(const std::vector<Token>& tokens,
                const Bindings& bindings,
                Value& value,
                std::string* pMsg = nullptr);

bool parseBindings(const std::vector<Token>& tokens,
                   Bindings& bindings,
                   std::string* pMsg = nullptr);

bool parseValueText(SymTable& symTable,
                    const std::string& text,
                    const Bindings& bindings,
                    Value& value,
                    std::string* pMsg = nullptr);

#endif
