#ifndef PARSEEXPR_HPP
#define PARSEEXPR_HPP

#include "Common.hpp"

class Token;
class Expr;

bool parseExpr(const std::vector<Token>& tokens,
               std::unique_ptr<Expr>* ppExpr,
               std::string* pMsg = nullptr);

#endif
