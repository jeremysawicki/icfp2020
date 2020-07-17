#include "VisitExpr.hpp"
#include "Expr.hpp"

using std::string;
using std::unique_ptr;

bool visitChildren(const VisitExprFn& fn,
                   unique_ptr<Expr>& pExpr,
                   string* pMsg)
{
    switch (pExpr->m_exprType)
    {
    case ExprType::Apply:
    {
        if (!fn(pExpr->m_applyData.m_funcExpr, pMsg)) return false;
        if (!fn(pExpr->m_applyData.m_argExpr, pMsg)) return false;
        break;
    }
    case ExprType::Lambda:
    {
        if (!fn(pExpr->m_lambdaData.m_bodyExpr, pMsg)) return false;
        break;
    }
    case ExprType::If:
    {
        if (!fn(pExpr->m_ifData.m_condExpr, pMsg)) return false;
        if (!fn(pExpr->m_ifData.m_trueExpr, pMsg)) return false;
        if (!fn(pExpr->m_ifData.m_falseExpr, pMsg)) return false;
        break;
    }
    case ExprType::And:
    {
        if (!fn(pExpr->m_andData.m_exprA, pMsg)) return false;
        if (!fn(pExpr->m_andData.m_exprB, pMsg)) return false;
        break;
    }
    case ExprType::Or:
    {
        if (!fn(pExpr->m_orData.m_exprA, pMsg)) return false;
        if (!fn(pExpr->m_orData.m_exprB, pMsg)) return false;
        break;
    }
    case ExprType::Builtin:
    {
        for (auto& argExpr : pExpr->m_builtinData.m_argExprs)
        {
            if (!fn(argExpr, pMsg)) return false;
        }
        break;
    }
    case ExprType::Let:
    {
        if (!fn(pExpr->m_letData.m_valExpr, pMsg)) return false;
        if (!fn(pExpr->m_letData.m_bodyExpr, pMsg)) return false;
        break;
    }
    case ExprType::Paren:
    {
        for (auto& subExpr : pExpr->m_parenData.m_subExprs)
        {
            if (!fn(subExpr, pMsg)) return false;
        }
        break;
    }
    default: ;
    }

    return true;
}
