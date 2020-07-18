#ifndef PARSEEXPR_HPP
#define PARSEEXPR_HPP

#include "Common.hpp"

class Token;
class Bindings;
class Value;

bool parseExpr(const std::vector<Token>& tokens,
               const Bindings& bindings,
               Value& value,
               std::string* pMsg = nullptr);

bool parseBindings(const std::vector<Token>& tokens,
                   Bindings& bindings,
                   std::string* pMsg = nullptr);

#endif
