#include "FormatExpr.hpp"
#include "Expr.hpp"
#include "Token.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

namespace
{
    bool formatExprImpl(const unique_ptr<Expr>& pExpr,
                        vector<Token>& tokens,
                        string* pMsg)
    {
        switch (pExpr->m_exprType)
        {
        case ExprType::Apply:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Apply);

            if (!formatExprImpl(pExpr->m_applyData.m_funcExpr, tokens, pMsg))
            {
                return false;
            }

            if (!formatExprImpl(pExpr->m_applyData.m_argExpr, tokens, pMsg))
            {
                return false;
            }

            break;
        }
        case ExprType::Lambda:
        {
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Lambda);
            }

            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Variable);
                token.m_variableData.m_varId = pExpr->m_lambdaData.m_varId;
            }

            if (!formatExprImpl(pExpr->m_lambdaData.m_bodyExpr, tokens, pMsg))
            {
                return false;
            }

            break;
        }
        case ExprType::If:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::If);

            if (!formatExprImpl(pExpr->m_ifData.m_condExpr, tokens, pMsg))
            {
                return false;
            }

            if (!formatExprImpl(pExpr->m_ifData.m_trueExpr, tokens, pMsg))
            {
                return false;
            }

            if (!formatExprImpl(pExpr->m_ifData.m_falseExpr, tokens, pMsg))
            {
                return false;
            }

            break;
        }
        case ExprType::And:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::And);

            if (!formatExprImpl(pExpr->m_andData.m_exprA, tokens, pMsg))
            {
                return false;
            }

            if (!formatExprImpl(pExpr->m_andData.m_exprB, tokens, pMsg))
            {
                return false;
            }

            break;
        }
        case ExprType::Or:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Or);

            if (!formatExprImpl(pExpr->m_orData.m_exprA, tokens, pMsg))
            {
                return false;
            }

            if (!formatExprImpl(pExpr->m_orData.m_exprB, tokens, pMsg))
            {
                return false;
            }

            break;
        }
        case ExprType::Integer:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Integer);
            token.m_integerData.m_value = pExpr->m_integerData.m_value;
            break;
        }
        case ExprType::Boolean:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Boolean);
            token.m_booleanData.m_value = pExpr->m_booleanData.m_value;
            break;
        }
        case ExprType::Variable:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Variable);
            token.m_variableData.m_varId = pExpr->m_variableData.m_varId;
            break;
        }
        case ExprType::Function:
        {
            auto& token = tokens.emplace_back();
            token.setTokenType(TokenType::Function);
            token.m_functionData.m_func = pExpr->m_functionData.m_func;
            break;
        }
        case ExprType::Let:
        {
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Let);
            }

            for (uint32_t varId : pExpr->m_letData.m_varIds)
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Variable);
                token.m_variableData.m_varId = varId;
            }

            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::Assign);
            }

            if (!formatExprImpl(pExpr->m_letData.m_valExpr, tokens, pMsg))
            {
                return false;
            }

            if (!formatExprImpl(pExpr->m_letData.m_bodyExpr, tokens, pMsg))
            {
                return false;
            }

            break;
        }
        case ExprType::Paren:
        {
            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::LParen);
            }

            for (auto& pSubExpr : pExpr->m_parenData.m_subExprs)
            {
                if (!formatExprImpl(pSubExpr, tokens, pMsg))
                {
                    return false;
                }
            }

            {
                auto& token = tokens.emplace_back();
                token.setTokenType(TokenType::RParen);
            }

            break;
        }
        default:
        {
            if (pMsg) *pMsg = "Unexpected expression type";
            return false;
        }
        }

        return true;
    }
}

bool formatExpr(const unique_ptr<Expr>& pExpr,
                vector<Token>* pTokens,
                string* pMsg)
{
    vector<Token> tokens;
    if (!formatExprImpl(pExpr, tokens, pMsg))
    {
        return false;
    }

    if (pTokens) *pTokens = std::move(tokens);
    return true;
}
