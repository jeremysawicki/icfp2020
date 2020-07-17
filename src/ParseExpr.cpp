#include "ParseExpr.hpp"
#include "Token.hpp"
#include "Expr.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

#define DEBUG 0

namespace
{
    bool parseExprImpl(const vector<Token>& tokens,
                       size_t& pos,
                       unique_ptr<Expr>& pExpr,
                       string* pMsg)
    {
#if DEBUG
        printf("> parseExprImpl\n");
#endif
        size_t size = tokens.size();
        if (pos >= size)
        {
            if (pMsg) *pMsg = "Unexpected end of input";
            return false;
        }

        auto& token = tokens[pos++];
        pExpr.reset(new Expr);

        switch (token.m_tokenType)
        {
        case TokenType::Apply:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Apply\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Apply);

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_applyData.m_funcExpr,
                               pMsg))
            {
                return false;
            }

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_applyData.m_argExpr,
                               pMsg))
            {
                return false;
            }

            break;
        }
        case TokenType::Lambda:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Lambda\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Lambda);

            if (pos >= size)
            {
                if (pMsg) *pMsg = "Unexpected end of input";
                return false;
            }

            auto& varToken = tokens[pos++];
            if (varToken.m_tokenType != TokenType::Variable)
            {
                if (pMsg) *pMsg = "Expected variable after lambda";
                return false;
            }

            pExpr->m_lambdaData.m_varId = varToken.m_variableData.m_varId;

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_lambdaData.m_bodyExpr,
                               pMsg))
            {
                return false;
            }

            break;
        }
        case TokenType::If:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::If\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::If);

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_ifData.m_condExpr,
                               pMsg))
            {
                return false;
            }

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_ifData.m_trueExpr,
                               pMsg))
            {
                return false;
            }

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_ifData.m_falseExpr,
                               pMsg))
            {
                return false;
            }

            break;
        }
        case TokenType::And:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::And\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::And);

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_andData.m_exprA,
                               pMsg))
            {
                return false;
            }

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_andData.m_exprB,
                               pMsg))
            {
                return false;
            }

            break;
        }
        case TokenType::Or:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Or\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Or);

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_orData.m_exprA,
                               pMsg))
            {
                return false;
            }

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_orData.m_exprB,
                               pMsg))
            {
                return false;
            }

            break;
        }
        case TokenType::Integer:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Integer\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Integer);
            pExpr->m_integerData.m_value = token.m_integerData.m_value;
            break;
        }
        case TokenType::Boolean:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Boolean\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Boolean);
            pExpr->m_booleanData.m_value = token.m_booleanData.m_value;
            break;
        }
        case TokenType::Variable:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Variable\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Variable);
            pExpr->m_variableData.m_varId = token.m_variableData.m_varId;
            break;
        }
        case TokenType::Function:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Function\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Function);
            pExpr->m_functionData.m_func = token.m_functionData.m_func;
            break;
        }
        case TokenType::Let:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Let\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Let);

            while (true)
            {
                if (pos >= size)
                {
                    if (pMsg) *pMsg = "Unexpected end of input";
                    return false;
                }

                auto& varToken = tokens[pos++];
                if (varToken.m_tokenType == TokenType::Variable)
                {
                    uint32_t varId = varToken.m_variableData.m_varId;
                    pExpr->m_letData.m_varIds.push_back(varId);
                }
                else if (varToken.m_tokenType == TokenType::Assign)
                {
                    break;
                }
                else
                {
                    if (pMsg) *pMsg = "Expected variable or = in let";
                    return false;
                }
            }

            if (pExpr->m_letData.m_varIds.empty())
            {
                if (pMsg) *pMsg = "Expected variable in let";
                return false;
            }

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_letData.m_valExpr,
                               pMsg))
            {
                return false;
            }

            if (!parseExprImpl(tokens,
                               pos,
                               pExpr->m_letData.m_bodyExpr,
                               pMsg))
            {
                return false;
            }

            break;
        }
        case TokenType::Assign:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::Assign\n", pos - 1);
#endif
            if (pMsg) *pMsg = "Unexpected = token";
            return false;
        }
        case TokenType::LParen:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::LParen\n", pos - 1);
#endif
            pExpr->setExprType(ExprType::Paren);

            while (true)
            {
                if (pos >= size)
                {
                    if (pMsg) *pMsg = "Unexpected end of input";
                    return false;
                }

                if (tokens[pos].m_tokenType == TokenType::RParen)
                {
                    pos++;
                    break;
                }

                auto& pSubExpr = pExpr->m_parenData.m_subExprs.emplace_back();
                if (!parseExprImpl(tokens,
                                   pos,
                                   pSubExpr,
                                   pMsg))
                {
                    return false;
                }
            }

            if (pExpr->m_parenData.m_subExprs.empty())
            {
                if (pMsg) *pMsg = "Expected subexpression in ()";
                return false;
            }

            break;
        }
        case TokenType::RParen:
        {
#if DEBUG
            printf("%" PRIuZ ": TokenType::RParen\n", pos - 1);
#endif
            if (pMsg) *pMsg = "Unexpected ) token";
            return false;
        }
        default:
        {
            if (pMsg) *pMsg = "Unexpected token type";
            return false;
        }
        }

#if DEBUG
        printf("< parseExprImpl\n");
#endif
        return true;
    }
}

bool parseExpr(const vector<Token>& tokens,
               unique_ptr<Expr>* ppExpr,
               string* pMsg)
{
    size_t pos = 0;
    unique_ptr<Expr> pExpr;
    if (!parseExprImpl(tokens,
                       pos,
                       pExpr,
                       pMsg))
    {
        return false;
    }

    if (pos != tokens.size())
    {
        if (pMsg) *pMsg = "Unexpected extra input";
        return false;
    }

    if (ppExpr) *ppExpr = std::move(pExpr);
    return true;
}
