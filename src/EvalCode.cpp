#include "EvalCode.hpp"
#include "Code.hpp"
#include "Inst.hpp"
#include "Value.hpp"
#include "Cleanup.hpp"

using std::string;
using std::vector;
using std::shared_ptr;

#define DEBUG 0

namespace
{
    bool evalCodeImpl(const Code& code,
                      uint32_t iInst,
                      Closure* pClosure,
                      Value boundValue,
                      vector<Value>& stack,
                      string* pMsg)
    {
        Cleanup cleanupClosure([&](){ if (pClosure) pClosure->release(); });

        while (true)
        {
            auto& inst = code.m_insts[iInst++];

            switch (inst.m_instType)
            {
            case InstType::Call:
            case InstType::TailCall:
            {
#if DEBUG
                if (inst.m_instType == InstType::TailCall)
                {
                    printf("%" PRIu32 ": InstType::TailCall\n", iInst - 1);
                }
                else
                {
                    printf("%" PRIu32 ": InstType::Call\n", iInst - 1);
                }
#endif
                Value argValue = std::move(stack.back());
                stack.pop_back();

                Value funcValue = std::move(stack.back());
                stack.pop_back();

                if (funcValue.m_valueType != ValueType::Closure)
                {
                    if (pMsg) *pMsg = "Attempt to call something other than a function";
                    return false;
                }

                Closure* pFuncClosure = funcValue.m_closureData.m_pClosure;

                if (pFuncClosure) pFuncClosure->addRef();

                if (inst.m_instType == InstType::TailCall)
                {
                    iInst = pFuncClosure->m_iInst;
                    if (pClosure) pClosure->release();
                    pClosure = pFuncClosure;
                    boundValue = std::move(argValue);
                }
                else
                {
                    if (!evalCodeImpl(code,
                                      pFuncClosure->m_iInst,
                                      pFuncClosure,
                                      argValue,
                                      stack,
                                      pMsg))
                    {
                        return false;
                    }
                }

                break;
            }
            case InstType::Lambda:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Lambda\n", iInst - 1);
#endif
                auto& value = stack.emplace_back();
                value.setValueType(ValueType::Closure);
                uint32_t iEntry = inst.m_lambdaData.m_iEntry;
                uint32_t startSlot = code.m_entries[iEntry].m_iSlot;
                uint32_t endSlot = code.m_entries[iEntry + 1].m_iSlot;
                uint32_t slotCount = endSlot - startSlot;
                value.m_closureData.m_pClosure = Closure::create(slotCount);
                for (uint32_t slot = 0; slot < slotCount; slot++)
                {
                    uint32_t oldSlot = code.m_slotMap[startSlot + slot];
                    if (oldSlot == (uint32_t)-1)
                    {
                        value.m_closureData.m_pClosure->m_freeEnv[slot] = boundValue;
                    }
                    else
                    {
                        value.m_closureData.m_pClosure->m_freeEnv[slot] = pClosure->m_freeEnv[oldSlot];
                    }
                }
                value.m_closureData.m_pClosure->m_iInst = code.m_entries[iEntry].m_iInst;
                break;
            }
            case InstType::Jf:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Jf\n", iInst - 1);
#endif
                Value condValue = std::move(stack.back());
                stack.pop_back();
                if (condValue.m_valueType != ValueType::Boolean)
                {
                    if (pMsg) *pMsg = "Condition not boolean";
                    return false;
                }

                if (!condValue.m_booleanData.m_value)
                {
                    iInst = inst.m_jfData.m_iInst;
                }

                break;
            }
            case InstType::Jmp:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Jmp\n", iInst - 1);
#endif
                iInst = inst.m_jmpData.m_iInst;

                break;
            }
            case InstType::Integer:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Integer\n", iInst - 1);
#endif
                auto& value = stack.emplace_back();
                value.setValueType(ValueType::Integer);
                value.m_integerData.m_value = inst.m_integerData.m_value;
                break;
            }
            case InstType::Boolean:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Boolean\n", iInst - 1);
#endif
                auto& value = stack.emplace_back();
                value.setValueType(ValueType::Boolean);
                value.m_booleanData.m_value = inst.m_booleanData.m_value;
                break;
            }
            case InstType::Variable:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Variable\n", iInst - 1);
#endif
                auto& value = stack.emplace_back();
                if (inst.m_variableData.m_slot == (uint32_t)-1)
                {
                    value = boundValue;
                }
                else
                {
                    value = pClosure->m_freeEnv[inst.m_variableData.m_slot];
                }
                break;
            }
            case InstType::Return:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Return\n", iInst - 1);
#endif
                return true;
            }
            case InstType::Inc:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Inc\n", iInst - 1);
#endif
                auto& valueA = stack.back();
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& a = valueA.m_integerData.m_value;

                if (!Int::inc(a, a, pMsg))
                {
                    return false;
                }

                break;
            }
            case InstType::Dec:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Dec\n", iInst - 1);
#endif
                auto& valueA = stack.back();
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& a = valueA.m_integerData.m_value;

                if (!Int::dec(a, a, pMsg))
                {
                    return false;
                }

                break;
            }
            case InstType::Neg:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Neg\n", iInst - 1);
#endif
                auto& valueA = stack.back();
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& a = valueA.m_integerData.m_value;

                if (!Int::neg(a, a, pMsg))
                {
                    return false;
                }

                break;
            }
            case InstType::Not:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Not\n", iInst - 1);
#endif
                auto& valueA = stack.back();
                if (valueA.m_valueType != ValueType::Boolean)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                bool& a = valueA.m_booleanData.m_value;

                a = !a;

                break;
            }
            case InstType::Add:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Add\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& a = valueA.m_integerData.m_value;

                if (!Int::add(a, a, b, pMsg)) return false;

                stack.pop_back();

                break;
            }
            case InstType::Sub:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Sub\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& a = valueA.m_integerData.m_value;

                if (!Int::sub(a, a, b, pMsg)) return false;

                stack.pop_back();

                break;
            }
            case InstType::Mul:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Mul\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& a = valueA.m_integerData.m_value;

                if (!Int::mul(a, a, b, pMsg)) return false;

                stack.pop_back();

                break;
            }
            case InstType::Div:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Div\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& a = valueA.m_integerData.m_value;

                if (!Int::div(a, a, b, pMsg)) return false;

                stack.pop_back();

                break;
            }
            case InstType::Eq:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Eq\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int a = std::move(valueA.m_integerData.m_value);

                valueA.setValueType(ValueType::Boolean);
                bool& r = valueA.m_booleanData.m_value;

                r = Int::eq(a, b);

                stack.pop_back();

                break;
            }
            case InstType::Ne:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Ne\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int a = std::move(valueA.m_integerData.m_value);

                valueA.setValueType(ValueType::Boolean);
                bool& r = valueA.m_booleanData.m_value;

                r = Int::ne(a, b);

                stack.pop_back();

                break;
            }
            case InstType::Lt:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Lt\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int a = std::move(valueA.m_integerData.m_value);

                valueA.setValueType(ValueType::Boolean);
                bool& r = valueA.m_booleanData.m_value;

                r = Int::lt(a, b);

                stack.pop_back();

                break;
            }
            case InstType::Gt:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Gt\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int a = std::move(valueA.m_integerData.m_value);

                valueA.setValueType(ValueType::Boolean);
                bool& r = valueA.m_booleanData.m_value;

                r = Int::gt(a, b);

                stack.pop_back();

                break;
            }
            case InstType::Le:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Le\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int a = std::move(valueA.m_integerData.m_value);

                valueA.setValueType(ValueType::Boolean);
                bool& r = valueA.m_booleanData.m_value;

                r = Int::le(a, b);

                stack.pop_back();

                break;
            }
            case InstType::Ge:
            {
#if DEBUG
                printf("%" PRIu32 ": InstType::Ge\n", iInst - 1);
#endif
                auto& valueB = stack.back();
                if (valueB.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int& b = valueB.m_integerData.m_value;

                auto& valueA = *(stack.end() - 2);
                if (valueA.m_valueType != ValueType::Integer)
                {
                    if (pMsg) *pMsg = "Bad argument type";
                    return false;
                }
                Int a = std::move(valueA.m_integerData.m_value);

                valueA.setValueType(ValueType::Boolean);
                bool& r = valueA.m_booleanData.m_value;

                r = Int::ge(a, b);

                stack.pop_back();

                break;
            }
            default:
            {
                printf("instType = %" PRIu32 "\n", (uint32_t)inst.m_instType);
                if (pMsg) *pMsg = "Unexpected instruction type";
                return false;
            }
            }
        }
    }
}

bool evalCode(const Code& code,
              Value* pValue,
              string* pMsg)
{
    vector<Value> stack;
    if (!evalCodeImpl(code,
                      0,
                      nullptr,
                      Value(),
                      stack,
                      pMsg))
    {
        return false;
    }

    if (pValue) *pValue = std::move(stack.back());
    return true;
}
