#ifndef OPTIMIZEEXPR_HPP
#define OPTIMIZEEXPR_HPP

#include "Common.hpp"

class Expr;

bool optimizeExpr(std::unique_ptr<Expr>& pExpr,
                  std::string* pMsg = nullptr);

#endif
