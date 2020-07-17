#ifndef PARSECODE_HPP
#define PARSECODE_HPP

#include "Common.hpp"

class Expr;
class Code;

bool parseCode(const std::unique_ptr<Expr>& pExpr,
               Code* pCode,
               std::string* pMsg = nullptr);

#endif
