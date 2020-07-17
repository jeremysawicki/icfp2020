#ifndef DESUGAREXPR_HPP
#define DESUGAREXPR_HPP

#include "Common.hpp"

class Expr;

bool desugarExpr(std::unique_ptr<Expr>& pExpr,
                 std::string* pMsg = nullptr);

#endif
