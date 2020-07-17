#include "OptimizeExpr.hpp"
#include "Expr.hpp"
#include "VisitExpr.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

namespace
{
    bool optimizeBuiltins(unique_ptr<Expr>& pExpr,
                          string* pMsg)
    {
        // ap f a -> _f a
        if (pExpr->m_exprType == ExprType::Apply)
        {
            auto& pExpr2 = pExpr->m_applyData.m_funcExpr;
            if (pExpr2->m_exprType == ExprType::Function)
            {
                Function func = pExpr2->m_functionData.m_func;
                if (func == Function::Inc ||
                    func == Function::Dec ||
                    func == Function::Neg ||
                    func == Function::Not)
                {
                    vector<unique_ptr<Expr>> argExprs;
                    argExprs.push_back(std::move(pExpr->m_applyData.m_argExpr));
                    
                    pExpr->setExprType(ExprType::Builtin);
                    pExpr->m_builtinData.m_func = func;
                    pExpr->m_builtinData.m_argExprs = std::move(argExprs);
                }
            }
        }

        // ap ap f a b -> _f a b
        if (pExpr->m_exprType == ExprType::Apply)
        {
            auto& pExpr2 = pExpr->m_applyData.m_funcExpr;
            if (pExpr2->m_exprType == ExprType::Apply)
            {
                auto& pExpr3 = pExpr2->m_applyData.m_funcExpr;
                if (pExpr3->m_exprType == ExprType::Function)
                {
                    Function func = pExpr3->m_functionData.m_func;
                    if (func == Function::Add ||
                        func == Function::Sub ||
                        func == Function::Mul ||
                        func == Function::Div ||
                        func == Function::Eq ||
                        func == Function::Ne ||
                        func == Function::Lt ||
                        func == Function::Gt ||
                        func == Function::Le ||
                        func == Function::Ge)
                    {
                        vector<unique_ptr<Expr>> argExprs;
                        argExprs.push_back(std::move(pExpr2->m_applyData.m_argExpr));
                        argExprs.push_back(std::move(pExpr->m_applyData.m_argExpr));
                    
                        pExpr->setExprType(ExprType::Builtin);
                        pExpr->m_builtinData.m_func = func;
                        pExpr->m_builtinData.m_argExprs = std::move(argExprs);
                    }
                }
            }
        }

        // f -> \a _f a
        // f -> \a \b _f a b
        if (pExpr->m_exprType == ExprType::Function)
        {
            Function func = pExpr->m_functionData.m_func;
            if (func == Function::Inc ||
                func == Function::Dec ||
                func == Function::Neg ||
                func == Function::Not)
            {
                pExpr->setExprType(ExprType::Lambda);
                pExpr->m_lambdaData.m_varId = 0;
                auto& pBuiltinExpr = pExpr->m_lambdaData.m_bodyExpr;
                pBuiltinExpr.reset(new Expr);
                pBuiltinExpr->setExprType(ExprType::Builtin);
                pBuiltinExpr->m_builtinData.m_func = func;
                pBuiltinExpr->m_builtinData.m_argExprs.resize(1);
                pBuiltinExpr->m_builtinData.m_argExprs[0].reset(new Expr);
                pBuiltinExpr->m_builtinData.m_argExprs[0]->setExprType(ExprType::Variable);
                pBuiltinExpr->m_builtinData.m_argExprs[0]->m_variableData.m_varId = 0;
            }
            else if (func == Function::Add ||
                     func == Function::Sub ||
                     func == Function::Mul ||
                     func == Function::Div ||
                     func == Function::Eq ||
                     func == Function::Ne ||
                     func == Function::Lt ||
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
                auto& pBuiltinExpr = pExpr2->m_lambdaData.m_bodyExpr;
                pBuiltinExpr.reset(new Expr);
                pBuiltinExpr->setExprType(ExprType::Builtin);
                pBuiltinExpr->m_builtinData.m_func = func;
                pBuiltinExpr->m_builtinData.m_argExprs.resize(2);
                pBuiltinExpr->m_builtinData.m_argExprs[0].reset(new Expr);
                pBuiltinExpr->m_builtinData.m_argExprs[0]->setExprType(ExprType::Variable);
                pBuiltinExpr->m_builtinData.m_argExprs[0]->m_variableData.m_varId = 0;
                pBuiltinExpr->m_builtinData.m_argExprs[1].reset(new Expr);
                pBuiltinExpr->m_builtinData.m_argExprs[1]->setExprType(ExprType::Variable);
                pBuiltinExpr->m_builtinData.m_argExprs[1]->m_variableData.m_varId = 1;
            }
            else
            {
                if (pMsg) *pMsg = "Unexpected function converting to builtin";
                return false;
            }
        }

        if (!visitChildren(optimizeBuiltins, pExpr, pMsg)) return false;

        return true;
    }

    bool optimizeExtraFuncs(unique_ptr<Expr>& pExpr,
                            string* pMsg)
    {
        if (!visitChildren(optimizeExtraFuncs, pExpr, pMsg)) return false;

        // (mul x -1) -> (neg x)
        // (mul -1 x) -> (neg x)
        if (pExpr->m_exprType == ExprType::Builtin &&
            pExpr->m_builtinData.m_func == Function::Mul)
        {
            if (pExpr->m_builtinData.m_argExprs[1]->m_exprType == ExprType::Integer &&
                Int::eq(pExpr->m_builtinData.m_argExprs[1]->m_integerData.m_value, Int(-1)))
            {
                pExpr->m_builtinData.m_func = Function::Neg;
                pExpr->m_builtinData.m_argExprs.resize(1);
            }
            else if (pExpr->m_builtinData.m_argExprs[0]->m_exprType == ExprType::Integer &&
                     Int::eq(pExpr->m_builtinData.m_argExprs[0]->m_integerData.m_value, Int(-1)))
            {
                pExpr->m_builtinData.m_func = Function::Neg;
                pExpr->m_builtinData.m_argExprs[0] = std::move(pExpr->m_builtinData.m_argExprs[1]);
                pExpr->m_builtinData.m_argExprs.resize(1);
            }
        }

        // (add x (neg y)) -> (sub x y)
        // (add (neg x) y) -> (sub y x)
        if (pExpr->m_exprType == ExprType::Builtin &&
            pExpr->m_builtinData.m_func == Function::Add)
        {
            if (pExpr->m_builtinData.m_argExprs[1]->m_exprType == ExprType::Builtin &&
                pExpr->m_builtinData.m_argExprs[1]->m_builtinData.m_func == Function::Neg)
            {
                pExpr->m_builtinData.m_func = Function::Sub;
                unique_ptr<Expr> pExprY = std::move(pExpr->m_builtinData.m_argExprs[1]->m_builtinData.m_argExprs[0]);
                pExpr->m_builtinData.m_argExprs[1] = std::move(pExprY);
            }
            else if (pExpr->m_builtinData.m_argExprs[0]->m_exprType == ExprType::Builtin &&
                     pExpr->m_builtinData.m_argExprs[0]->m_builtinData.m_func == Function::Neg)
            {
                pExpr->m_builtinData.m_func = Function::Sub;
                unique_ptr<Expr> pExprX = std::move(pExpr->m_builtinData.m_argExprs[0]->m_builtinData.m_argExprs[0]);
                pExpr->m_builtinData.m_argExprs[0] = std::move(pExpr->m_builtinData.m_argExprs[1]);
                pExpr->m_builtinData.m_argExprs[1] = std::move(pExprX);
            }
        }

        return true;
    }
}

bool optimizeExpr(unique_ptr<Expr>& pExpr,
                  string* pMsg)
{
    if (!optimizeBuiltins(pExpr, pMsg)) return false;
    if (!optimizeExtraFuncs(pExpr, pMsg)) return false;
    return true;
}
