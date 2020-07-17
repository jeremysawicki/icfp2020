#ifndef FORMATEXPR_HPP
#define FORMATEXPR_HPP

#include "Common.hpp"

class Expr;
class Token;

bool formatExpr(const std::unique_ptr<Expr>& pExpr,
                std::vector<Token>* pTokens,
                std::string* pMsg = nullptr);

#endif
