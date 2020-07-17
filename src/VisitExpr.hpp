#ifndef VISITEXPR_HPP
#define VISITEXPR_HPP

#include "Common.hpp"

class Expr;

typedef std::function<bool(std::unique_ptr<Expr>&, std::string*)> VisitExprFn;

bool visitChildren(const VisitExprFn& fn,
                   std::unique_ptr<Expr>& pExpr,
                   std::string* pMsg = nullptr);

#endif
