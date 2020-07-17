#include "ParseCode.hpp"
#include "Expr.hpp"
#include "Code.hpp"
#include "Inst.hpp"
#include "StringUtils.hpp"

using std::string;
using std::vector;
using std::unique_ptr;

#define DEBUG 0

namespace
{
    class Rec
    {
    public:
        vector<Inst> m_insts;
        vector<uint32_t> m_slotMap;
    };

    bool parseCodeImpl(const unique_ptr<Expr>& pExpr,
                       vector<Inst>& insts,
                       uint32_t boundVarId,
                       vector<uint32_t>& freeVarIds,
                       vector<Rec>& recs,
                       bool needReturn,
                       string* pMsg)
    {
#if DEBUG
        printf("> parseCodeImpl\n");
#endif

        auto addVar = [&](uint32_t varId) -> uint32_t
        {
            uint32_t varIdx = 0;
            if (varId == boundVarId)
            {
                varIdx = (uint32_t)-1;
            }
            else
            {
                auto varIt = std::find(freeVarIds.begin(), freeVarIds.end(), varId);
                if (varIt != freeVarIds.end())
                {
                    varIdx = (uint32_t)(varIt - freeVarIds.begin());
                }
                else
                {
                    varIdx = (uint32_t)freeVarIds.size();
                    freeVarIds.push_back(varId);
                }
            }
            return varIdx;
        };

        switch (pExpr->m_exprType)
        {
        case ExprType::Apply:
        {
#if DEBUG
            printf("ExprType::Apply\n");
#endif
            if (!parseCodeImpl(pExpr->m_applyData.m_funcExpr,
                               insts,
                               boundVarId,
                               freeVarIds,
                               recs,
                               false,
                               pMsg))
            {
                return false;
            }

            if (!parseCodeImpl(pExpr->m_applyData.m_argExpr,
                               insts,
                               boundVarId,
                               freeVarIds,
                               recs,
                               false,
                               pMsg))
            {
                return false;
            }

            auto& inst = insts.emplace_back();
            inst.setInstType(needReturn ? InstType::TailCall : InstType::Call);

            needReturn = false;

            break;
        }
        case ExprType::Lambda:
        {
#if DEBUG
            printf("ExprType::Lambda\n");
#endif
            uint32_t iLambdaEntry = recs.size();
            recs.emplace_back();

            auto& inst = insts.emplace_back();
            inst.setInstType(InstType::Lambda);
            inst.m_lambdaData.m_iEntry = iLambdaEntry;

            vector<Inst> lambdaInsts;
            vector<uint32_t> lambdaFreeVarIds;
            if (!parseCodeImpl(pExpr->m_lambdaData.m_bodyExpr,
                               lambdaInsts,
                               pExpr->m_lambdaData.m_varId,
                               lambdaFreeVarIds,
                               recs,
                               true,
                               pMsg))
            {
                return false;
            }

            auto& rec = recs[iLambdaEntry];
            rec.m_insts = std::move(lambdaInsts);
            rec.m_slotMap.resize(lambdaFreeVarIds.size());
            for (uint32_t slot = 0; slot < (uint32_t)lambdaFreeVarIds.size(); slot++)
            {
                uint32_t varIdx = addVar(lambdaFreeVarIds[slot]);
                rec.m_slotMap[slot] = varIdx;
            }

            break;
        }
        case ExprType::If:
        {
#if DEBUG
            printf("ExprType::If\n");
#endif
            if (!parseCodeImpl(pExpr->m_ifData.m_condExpr,
                               insts,
                               boundVarId,
                               freeVarIds,
                               recs,
                               false,
                               pMsg))
            {
                return false;
            }

            uint32_t iJfInst = insts.size();
            {
                auto& inst = insts.emplace_back();
                inst.setInstType(InstType::Jf);
            }

            if (!parseCodeImpl(pExpr->m_ifData.m_trueExpr,
                               insts,
                               boundVarId,
                               freeVarIds,
                               recs,
                               needReturn,
                               pMsg))
            {
                return false;
            }

            uint32_t iJmpInst = insts.size();
            if (!needReturn)
            {
                auto& inst = insts.emplace_back();
                inst.setInstType(InstType::Jmp);
            }
            insts[iJfInst].m_jfData.m_iInst = insts.size();

            if (!parseCodeImpl(pExpr->m_ifData.m_falseExpr,
                               insts,
                               boundVarId,
                               freeVarIds,
                               recs,
                               needReturn,
                               pMsg))
            {
                return false;
            }

            if (!needReturn)
            {
                insts[iJmpInst].m_jmpData.m_iInst = insts.size();
            }

            needReturn = false;

            break;
        }
        case ExprType::Integer:
        {
#if DEBUG
            printf("ExprType::Integer\n");
#endif

            auto& inst = insts.emplace_back();
            inst.setInstType(InstType::Integer);
            inst.m_integerData.m_value = pExpr->m_integerData.m_value;

            break;
        }
        case ExprType::Boolean:
        {
#if DEBUG
            printf("ExprType::Boolean\n");
#endif

            auto& inst = insts.emplace_back();
            inst.setInstType(InstType::Boolean);
            inst.m_booleanData.m_value = pExpr->m_booleanData.m_value;

            break;
        }
        case ExprType::Variable:
        {
#if DEBUG
            printf("ExprType::Variable\n");
#endif

            auto& inst = insts.emplace_back();
            inst.setInstType(InstType::Variable);
            uint32_t varIdx = addVar(pExpr->m_variableData.m_varId);
            inst.m_variableData.m_slot = varIdx;

            break;
        }
        case ExprType::Function:
        {
#if DEBUG
            printf("ExprType::Function\n");
#endif

            if (pMsg) *pMsg = "Expected function to be converted to builtin by now";
            return false;
        }
        case ExprType::Builtin:
        {
#if DEBUG
            printf("ExprType::Builtin\n");
#endif

            for (auto& argExpr : pExpr->m_builtinData.m_argExprs)
            {
                if (!parseCodeImpl(argExpr,
                                   insts,
                                   boundVarId,
                                   freeVarIds,
                                   recs,
                                   false,
                                   pMsg))
                {
                    return false;
                }
            }

            auto& inst = insts.emplace_back();

            switch (pExpr->m_builtinData.m_func)
            {
            case Function::Inc: inst.setInstType(InstType::Inc); break;
            case Function::Dec: inst.setInstType(InstType::Dec); break;
            case Function::Neg: inst.setInstType(InstType::Neg); break;
            case Function::Not: inst.setInstType(InstType::Not); break;
            case Function::Add: inst.setInstType(InstType::Add); break;
            case Function::Sub: inst.setInstType(InstType::Sub); break;
            case Function::Mul: inst.setInstType(InstType::Mul); break;
            case Function::Div: inst.setInstType(InstType::Div); break;
            case Function::Eq: inst.setInstType(InstType::Eq); break;
            case Function::Ne: inst.setInstType(InstType::Ne); break;
            case Function::Lt: inst.setInstType(InstType::Lt); break;
            case Function::Gt: inst.setInstType(InstType::Gt); break;
            case Function::Le: inst.setInstType(InstType::Le); break;
            case Function::Ge: inst.setInstType(InstType::Ge); break;
            default:
                if (pMsg) *pMsg = "Unexpected builtin function type";
                return false;
            }

            break;
        }
        default:
        {
            if (pMsg) *pMsg = "Unexpected expression type";
            return false;
        }
        }

        if (needReturn)
        {
            auto& inst = insts.emplace_back();
            inst.setInstType(InstType::Return);
        }

#if DEBUG
        printf("< parseCodeImpl\n");
#endif
        return true;
    }
}

bool parseCode(const unique_ptr<Expr>& pExpr,
               Code* pCode,
               string* pMsg)
{
    vector<Rec> recs;
    recs.emplace_back();

    vector<Inst> insts;
    vector<uint32_t> freeVarIds;
    if (!parseCodeImpl(pExpr,
                       insts,
                       (uint32_t)-1,
                       freeVarIds,
                       recs,
                       true,
                       pMsg))
    {
        return false;
    }

    if (!freeVarIds.empty())
    {
        if (pMsg)
        {
            *pMsg = "Unexpected free variables: ";
            for (size_t i = 0; i < freeVarIds.size(); i++)
            {
                if (i > 0)
                {
                    *pMsg += ", ";
                }
                *pMsg += strprintf("%" PRIu32 "", freeVarIds[i]);
            }
        }
        return false;
    }

    auto& rec = recs[0];
    rec.m_insts = std::move(insts);

    uint32_t entryCount = recs.size();
    uint32_t instCount = 0;
    uint32_t slotCount = 0;
    for (uint32_t iEntry = 0; iEntry < entryCount; iEntry++)
    {
        auto& rec = recs[iEntry];
        instCount += rec.m_insts.size();
        slotCount += rec.m_slotMap.size();
    }
    pCode->m_entries.resize(entryCount + 1);
    pCode->m_insts.resize(instCount);
    pCode->m_slotMap.resize(slotCount);
    pCode->m_entries[0].m_iInst = 0;
    pCode->m_entries[0].m_iSlot = 0;
    uint32_t iInst = 0;
    uint32_t iSlot = 0;
    for (uint32_t iEntry = 0; iEntry < entryCount; iEntry++)
    {
        auto& rec = recs[iEntry];
        uint32_t entryInstCount = rec.m_insts.size();
        uint32_t offset = iInst;
        for (uint32_t iEntryInst = 0; iEntryInst < entryInstCount; iEntryInst++)
        {
            auto& inst = pCode->m_insts[iInst++];
            inst = rec.m_insts[iEntryInst];
            switch (inst.m_instType)
            {
            case InstType::Jf: inst.m_jfData.m_iInst += offset; break;
            case InstType::Jmp: inst.m_jmpData.m_iInst += offset; break;
            default: ;
            }
        }
        uint32_t entrySlotCount = rec.m_slotMap.size();
        for (uint32_t iEntrySlot = 0; iEntrySlot < entrySlotCount; iEntrySlot++)
        {
            pCode->m_slotMap[iSlot++] = rec.m_slotMap[iEntrySlot];
        }
        auto& entry = pCode->m_entries[iEntry + 1];
        entry.m_iInst = iInst;
        entry.m_iSlot = iSlot;
    }

    return true;
}
