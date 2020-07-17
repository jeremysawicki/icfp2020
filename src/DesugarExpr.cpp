#include "DesugarExpr.hpp"
#include "Expr.hpp"
#include "VisitExpr.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

namespace
{
    bool desugarLet(unique_ptr<Expr>& pExpr,
                    string* pMsg)
    {
        if (!visitChildren(desugarLet, pExpr, pMsg)) return false;

        if (pExpr->m_exprType == ExprType::Let)
        {
            if (pExpr->m_letData.m_varIds.empty())
            {
                if (pMsg) *pMsg = "Malformed let";
                return false;
            }

            vector<uint32_t> varIds(std::move(pExpr->m_letData.m_varIds));
            unique_ptr<Expr> pValExpr(std::move(pExpr->m_letData.m_valExpr));
            unique_ptr<Expr> pBodyExpr(std::move(pExpr->m_letData.m_bodyExpr));

            unique_ptr<Expr> pFuncExpr(new Expr);
            pFuncExpr->setExprType(ExprType::Lambda);
            pFuncExpr->m_lambdaData.m_varId = varIds[0];
            pFuncExpr->m_lambdaData.m_bodyExpr = std::move(pBodyExpr);

            unique_ptr<Expr> pArgExpr(std::move(pValExpr));
            for (size_t iVar = varIds.size(); iVar > 1; )
            {
                iVar--;
                uint32_t varId = varIds[iVar];
                unique_ptr<Expr> pLambdaExpr(new Expr);
                pLambdaExpr->setExprType(ExprType::Lambda);
                pLambdaExpr->m_lambdaData.m_varId = varId;
                pLambdaExpr->m_lambdaData.m_bodyExpr = std::move(pArgExpr);
                pArgExpr = std::move(pLambdaExpr);
            }

            pExpr->setExprType(ExprType::Apply);
            pExpr->m_applyData.m_funcExpr = std::move(pFuncExpr);
            pExpr->m_applyData.m_argExpr = std::move(pArgExpr);
        }

        return true;
    }

    bool desugarParen(unique_ptr<Expr>& pExpr,
                      string* pMsg)
    {
        if (!visitChildren(desugarParen, pExpr, pMsg)) return false;

        if (pExpr->m_exprType == ExprType::Paren)
        {
            if (pExpr->m_parenData.m_subExprs.empty())
            {
                if (pMsg) *pMsg = "No subexpressions in ()";
                return false;
            }

            vector<unique_ptr<Expr>> subExprs(std::move(pExpr->m_parenData.m_subExprs));

            pExpr = std::move(subExprs[0]);

            for (size_t iArg = 1; iArg < subExprs.size(); iArg++)
            {
                unique_ptr<Expr> pApplyExpr(new Expr);
                pApplyExpr->setExprType(ExprType::Apply);
                pApplyExpr->m_applyData.m_funcExpr = std::move(pExpr);
                pApplyExpr->m_applyData.m_argExpr = std::move(subExprs[iArg]);
                pExpr = std::move(pApplyExpr);
            }
        }

        return true;
    }

    bool desugarAndOr(unique_ptr<Expr>& pExpr,
                      string* pMsg)
    {
        if (!visitChildren(desugarAndOr, pExpr, pMsg)) return false;

        // and a b -> if a b false
        if (pExpr->m_exprType == ExprType::And)
        {
            unique_ptr<Expr> exprA = std::move(pExpr->m_andData.m_exprA);
            unique_ptr<Expr> exprB = std::move(pExpr->m_andData.m_exprB);
            pExpr->setExprType(ExprType::If);
            pExpr->m_ifData.m_condExpr = std::move(exprA);
            pExpr->m_ifData.m_trueExpr = std::move(exprB);
            pExpr->m_ifData.m_falseExpr.reset(new Expr);
            pExpr->m_ifData.m_falseExpr->setExprType(ExprType::Boolean);
            pExpr->m_ifData.m_falseExpr->m_booleanData.m_value = false;
        }

        // or a b -> if a true b
        if (pExpr->m_exprType == ExprType::Or)
        {
            unique_ptr<Expr> exprA = std::move(pExpr->m_andData.m_exprA);
            unique_ptr<Expr> exprB = std::move(pExpr->m_andData.m_exprB);
            pExpr->setExprType(ExprType::If);
            pExpr->m_ifData.m_condExpr = std::move(exprA);
            pExpr->m_ifData.m_trueExpr.reset(new Expr);
            pExpr->m_ifData.m_trueExpr->setExprType(ExprType::Boolean);
            pExpr->m_ifData.m_trueExpr->m_booleanData.m_value = true;
            pExpr->m_ifData.m_falseExpr = std::move(exprB);
        }

        return true;
    }

    bool desugarExtraFuncs(unique_ptr<Expr>& pExpr,
                           string* pMsg)
    {
        // f -> \a ap f a
        // f -> \a \b ap ap f a b
        if (pExpr->m_exprType == ExprType::Function)
        {
            Function func = pExpr->m_functionData.m_func;
            if (func == Function::Neg ||
                func == Function::Not)
            {
                pExpr->setExprType(ExprType::Lambda);
                pExpr->m_lambdaData.m_varId = 0;
                auto& pApplyExpr = pExpr->m_lambdaData.m_bodyExpr;
                pApplyExpr.reset(new Expr);
                pApplyExpr->setExprType(ExprType::Apply);
                pApplyExpr->m_applyData.m_funcExpr.reset(new Expr);
                pApplyExpr->m_applyData.m_funcExpr->setExprType(ExprType::Function);
                pApplyExpr->m_applyData.m_funcExpr->m_functionData.m_func = func;
                pApplyExpr->m_applyData.m_argExpr.reset(new Expr);
                pApplyExpr->m_applyData.m_argExpr->setExprType(ExprType::Variable);
                pApplyExpr->m_applyData.m_argExpr->m_variableData.m_varId = 0;
            }
            else if (func == Function::Sub ||
                     func == Function::Ne ||
                     func == Function::Gt ||
                     func == Function::Le ||
                     func == Function::Ge)
            {
                pExpr->setExprType(ExprType::Lambda);
                pExpr->m_lambdaData.m_varId = 0;
                auto& pExpr2 = pExpr->m_lambdaData.m_bodyExpr;
                pExpr2.reset(new Expr);
                pExpr2->setExprType(ExprType::Lambda);
                pExpr2->m_lambdaData.m_varId = 1;

                auto& pApplyExpr = pExpr2->m_lambdaData.m_bodyExpr;
                pApplyExpr.reset(new Expr);
                pApplyExpr->setExprType(ExprType::Apply);
                auto& pApplyExpr2 = pApplyExpr->m_applyData.m_funcExpr;
                pApplyExpr2.reset(new Expr);
                pApplyExpr2->setExprType(ExprType::Apply);
                
                pApplyExpr2->m_applyData.m_funcExpr.reset(new Expr);
                pApplyExpr2->m_applyData.m_funcExpr->setExprType(ExprType::Function);
                pApplyExpr2->m_applyData.m_funcExpr->m_functionData.m_func = func;
                pApplyExpr2->m_applyData.m_argExpr.reset(new Expr);
                pApplyExpr2->m_applyData.m_argExpr->setExprType(ExprType::Variable);
                pApplyExpr2->m_applyData.m_argExpr->m_variableData.m_varId = 0;
                pApplyExpr->m_applyData.m_argExpr.reset(new Expr);
                pApplyExpr->m_applyData.m_argExpr->setExprType(ExprType::Variable);
                pApplyExpr->m_applyData.m_argExpr->m_variableData.m_varId = 1;
            }
        }

        // (sub x y) -> (add x (neg y))
        // ap ap sub x y -> ap ap add x app neg y
        if (pExpr->m_exprType == ExprType::Apply)
        {
            auto& pExpr2 = pExpr->m_applyData.m_funcExpr;
            if (pExpr2->m_exprType == ExprType::Apply)
            {
                auto& pFuncExpr = pExpr2->m_applyData.m_funcExpr;
                if (pFuncExpr->m_exprType == ExprType::Function &&
                    pFuncExpr->m_functionData.m_func == Function::Sub)
                {
                    pFuncExpr->m_functionData.m_func = Function::Add;

                    auto& pArgExpr = pExpr->m_applyData.m_argExpr;
                    unique_ptr<Expr> pExprY = std::move(pArgExpr);

                    pArgExpr.reset(new Expr);
                    pArgExpr->setExprType(ExprType::Apply);
                    auto& pFuncExpr2 = pArgExpr->m_applyData.m_funcExpr;
                    auto& pArgExpr2 = pArgExpr->m_applyData.m_argExpr;

                    pFuncExpr2.reset(new Expr);
                    pFuncExpr2->setExprType(ExprType::Function);
                    pFuncExpr2->m_functionData.m_func = Function::Neg;

                    pArgExpr2 = std::move(pExprY);
                }
            }
        }

        // (neg x) -> (mul x -1)
        // ap neg x -> ap ap mul x -1
        if (pExpr->m_exprType == ExprType::Apply)
        {
            auto& pFuncExpr = pExpr->m_applyData.m_funcExpr;
            if (pFuncExpr->m_exprType == ExprType::Function &&
                pFuncExpr->m_functionData.m_func == Function::Neg)
            {
                auto& pArgExpr = pExpr->m_applyData.m_argExpr;
                unique_ptr<Expr> pExprX = std::move(pArgExpr);
                pArgExpr.reset(new Expr);
                pArgExpr->setExprType(ExprType::Integer);
                pArgExpr->m_integerData.m_value = Int(-1);

                pFuncExpr->setExprType(ExprType::Apply);
                auto& pFuncExpr2 = pFuncExpr->m_applyData.m_funcExpr;
                auto& pArgExpr2 = pFuncExpr->m_applyData.m_argExpr;

                pFuncExpr2.reset(new Expr);
                pFuncExpr2->setExprType(ExprType::Function);
                pFuncExpr2->m_functionData.m_func = Function::Mul;

                pArgExpr2 = std::move(pExprX);
            }
        }

        // (gt x y) -> (lt y x)
        // (le x y) -> (not (lt y x))
        // (ge x y) -> (not (lt x y))
        // (ne x y) -> (not (eq x y))
        // ap ap gt x y -> ap ap lt y x
        // ap ap le x y -> ap not ap ap lt y x
        // ap ap ge x y -> ap not ap ap lt x y
        // ap ap ne x y -> ap not ap ap eq x y
        if (pExpr->m_exprType == ExprType::Apply)
        {
            auto& pExpr2 = pExpr->m_applyData.m_funcExpr;
            if (pExpr2->m_exprType == ExprType::Apply)
            {
                auto& pFuncExpr = pExpr2->m_applyData.m_funcExpr;
                if (pFuncExpr->m_exprType == ExprType::Function)
                {
                    Function func = pFuncExpr->m_functionData.m_func;
                    if (func == Function::Gt ||
                        func == Function::Le ||
                        func == Function::Ge ||
                        func == Function::Ne)
                    {
                        if (func == Function::Ne)
                        {
                            pFuncExpr->m_functionData.m_func = Function::Eq;
                        }
                        else
                        {
                            pFuncExpr->m_functionData.m_func = Function::Lt;
                        }

                        if (func == Function::Gt ||
                            func == Function::Le)
                        {
                            pExpr->m_applyData.m_argExpr.swap(pExpr2->m_applyData.m_argExpr);
                        }
                        if (func == Function::Le ||
                            func == Function::Ge ||
                            func == Function::Ne)
                        {
                            unique_ptr<Expr> pNotExpr(new Expr);
                            pNotExpr->setExprType(ExprType::Apply);
                            pNotExpr->m_applyData.m_funcExpr.reset(new Expr);
                            pNotExpr->m_applyData.m_funcExpr->setExprType(ExprType::Function);
                            pNotExpr->m_applyData.m_funcExpr->m_functionData.m_func = Function::Not;
                            pNotExpr->m_applyData.m_argExpr = std::move(pExpr);
                            pExpr = std::move(pNotExpr);
                        }
                    }
                }
            }
        }

        // (not x) -> if x false true
        // ap neg x -> if x false true
        if (pExpr->m_exprType == ExprType::Apply)
        {
            auto& pFuncExpr = pExpr->m_applyData.m_funcExpr;
            if (pFuncExpr->m_exprType == ExprType::Function &&
                pFuncExpr->m_functionData.m_func == Function::Not)
            {
                auto& pArgExpr = pExpr->m_applyData.m_argExpr;
                unique_ptr<Expr> pExprX = std::move(pArgExpr);
                pExpr->setExprType(ExprType::If);
                pExpr->m_ifData.m_condExpr = std::move(pExprX);
                pExpr->m_ifData.m_trueExpr.reset(new Expr);
                pExpr->m_ifData.m_trueExpr->setExprType(ExprType::Boolean);
                pExpr->m_ifData.m_trueExpr->m_booleanData.m_value = false;
                pExpr->m_ifData.m_falseExpr.reset(new Expr);
                pExpr->m_ifData.m_falseExpr->setExprType(ExprType::Boolean);
                pExpr->m_ifData.m_falseExpr->m_booleanData.m_value = true;
            }
        }

        if (!visitChildren(desugarExtraFuncs, pExpr, pMsg)) return false;

        return true;
    }
}

bool desugarExpr(unique_ptr<Expr>& pExpr,
                 string* pMsg)
{
    if (!desugarLet(pExpr, pMsg)) return false;
    if (!desugarParen(pExpr, pMsg)) return false;
    if (!desugarAndOr(pExpr, pMsg)) return false;
    if (!desugarExtraFuncs(pExpr, pMsg)) return false;
    return true;
}
