#include "Eval.hpp"
#include "Value.hpp"
#include "Modem.hpp"

using std::string;
using std::vector;
using std::pair;

bool eval(Value& value,
          string* pMsg)
{
    while (value->m_valueType == ValueType::Apply)
    {
        Value funcValue = value->m_applyData.m_funcValue;
        Value argValue = value->m_applyData.m_argValue;
        value->setValueType(ValueType::Invalid);

        if (!eval(funcValue, pMsg))
        {
            return false;
        }

        if (funcValue->m_valueType != ValueType::Closure)
        {
            if (pMsg) *pMsg = "Attempt to call something other than a function";
            return false;
        }

        Function func = funcValue->m_closureData.m_func;
        uint32_t size = funcValue->m_closureData.m_size;
        auto& args = funcValue->m_closureData.m_args;

        uint32_t needed = 0;
        switch (func)
        {
        case Function::Inc: needed = 1; break;
        case Function::Dec: needed = 1; break;
        case Function::Add: needed = 2; break;
        case Function::Mul: needed = 2; break;
        case Function::Div: needed = 2; break;
        case Function::Eq: needed = 2; break;
        case Function::Lt: needed = 2; break;
        case Function::Modulate: needed = 1; break;
        case Function::Demodulate: needed = 1; break;
        case Function::Send: needed = 1; break;
        case Function::Neg: needed = 1; break;
        case Function::S: needed = 3; break;
        case Function::C: needed = 3; break;
        case Function::B: needed = 3; break;
        case Function::True: needed = 2; break;
        case Function::False: needed = 2; break;
        case Function::I: needed = 1; break;
        case Function::Cons: needed = 3; break;
        case Function::Car: needed = 1; break;
        case Function::Cdr: needed = 1; break;
        case Function::Nil: needed = 1; break;
        case Function::IsNil: needed = 1; break;
        case Function::Vec: needed = 3; break;
        case Function::Draw: needed = 1; break;
        case Function::Checkerboard: needed = 2; break;
        case Function::MultipleDraw: needed = 1; break;
        case Function::If0: needed = 3; break;
        default:
        {
            if (pMsg) *pMsg = "Unexpected function in closure";
            return false;
        }
        }

        if (size + 1 < needed)
        {
            value->setValueType(ValueType::Closure);
            value->m_closureData.m_func = func;
            value->m_closureData.m_size = size + 1;
            for (uint32_t i = 0; i < size; i++)
            {
                value->m_closureData.m_args[i] = args[i];
            }
            value->m_closureData.m_args[size] = argValue;
            return true;
        }

        switch (func)
        {
        case Function::Inc:
        {
            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = argValue->m_integerData.m_value;

            value->setValueType(ValueType::Integer);
            Int& r = value->m_integerData.m_value;

            if (!Int::inc(r, a, pMsg))
            {
                return false;
            }

            return true;
        }
        case Function::Dec:
        {
            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = argValue->m_integerData.m_value;

            value->setValueType(ValueType::Integer);
            Int& r = value->m_integerData.m_value;

            if (!Int::dec(r, a, pMsg))
            {
                return false;
            }

            return true;
        }
        case Function::Add:
        {
            if (!eval(args[0], pMsg)) return false;
            if (args[0]->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = args[0]->m_integerData.m_value;

            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& b = argValue->m_integerData.m_value;

            value->setValueType(ValueType::Integer);
            Int& r = value->m_integerData.m_value;

            if (!Int::add(r, a, b, pMsg))
            {
                return false;
            }

            return true;
        }
        case Function::Mul:
        {
            if (!eval(args[0], pMsg)) return false;
            if (args[0]->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = args[0]->m_integerData.m_value;

            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& b = argValue->m_integerData.m_value;

            value->setValueType(ValueType::Integer);
            Int& r = value->m_integerData.m_value;

            if (!Int::mul(r, a, b, pMsg))
            {
                return false;
            }

            return true;
        }
        case Function::Div:
        {
            if (!eval(args[0], pMsg)) return false;
            if (args[0]->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = args[0]->m_integerData.m_value;

            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& b = argValue->m_integerData.m_value;

            value->setValueType(ValueType::Integer);
            Int& r = value->m_integerData.m_value;

            if (!Int::div(r, a, b, pMsg))
            {
                return false;
            }

            return true;
        }
        case Function::Eq:
        {
            if (!eval(args[0], pMsg)) return false;
            if (args[0]->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = args[0]->m_integerData.m_value;

            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& b = argValue->m_integerData.m_value;

            bool r = Int::eq(a, b);

            value->setValueType(ValueType::Closure);
            value->m_closureData.m_func = r ? Function::True : Function::False;

            return true;
        }
        case Function::Lt:
        {
            if (!eval(args[0], pMsg)) return false;
            if (args[0]->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = args[0]->m_integerData.m_value;

            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& b = argValue->m_integerData.m_value;

            bool r = Int::lt(a, b);

            value->setValueType(ValueType::Closure);
            value->m_closureData.m_func = r ? Function::True : Function::False;

            return true;
        }
        case Function::Modulate:
        {
            string signal;
            if (!modulate(argValue, signal, pMsg)) return false;
            value->setValueType(ValueType::Signal);
            value->m_signalData.m_signal = std::move(signal);
            return true;
        }
        case Function::Demodulate:
        {
            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Signal)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            if (!demodulate(argValue->m_signalData.m_signal, value, pMsg))
            {
                return false;
            }
            return true;
        }
        //case Function::Send:
        case Function::Neg:
        {
            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType != ValueType::Integer)
            {
                if (pMsg) *pMsg = "Bad argument type";
                return false;
            }
            const Int& a = argValue->m_integerData.m_value;

            value->setValueType(ValueType::Integer);
            Int& r = value->m_integerData.m_value;

            if (!Int::neg(r, a, pMsg))
            {
                return false;
            }

            return true;
        }
        case Function::S:
        {
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue.init(ValueType::Apply);
            value->m_applyData.m_funcValue->m_applyData.m_funcValue = args[0];
            value->m_applyData.m_funcValue->m_applyData.m_argValue = argValue;
            value->m_applyData.m_argValue.init(ValueType::Apply);
            value->m_applyData.m_argValue->m_applyData.m_funcValue = args[1];
            value->m_applyData.m_argValue->m_applyData.m_argValue = argValue;
            break;
        }
        case Function::C:
        {
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue.init(ValueType::Apply);
            value->m_applyData.m_funcValue->m_applyData.m_funcValue = args[0];
            value->m_applyData.m_funcValue->m_applyData.m_argValue = argValue;
            value->m_applyData.m_argValue = args[1];
            break;
        }
        case Function::B:
        {
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue = args[0];
            value->m_applyData.m_argValue.init(ValueType::Apply);
            value->m_applyData.m_argValue->m_applyData.m_funcValue = args[1];
            value->m_applyData.m_argValue->m_applyData.m_argValue = argValue;
            break;
        }
        case Function::True:
        {
            value = args[0];
            break;
        }
        case Function::False:
        {
            value = argValue;
            break;
        }
        case Function::I:
        {
            value = argValue;
            break;
        }
        case Function::Cons:
        {
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue.init(ValueType::Apply);
            value->m_applyData.m_funcValue->m_applyData.m_funcValue = argValue;
            value->m_applyData.m_funcValue->m_applyData.m_argValue = args[0];
            value->m_applyData.m_argValue = args[1];
            break;
        }
        case Function::Car:
        {
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue = argValue;
            value->m_applyData.m_argValue.init(ValueType::Closure);
            value->m_applyData.m_argValue->m_closureData.m_func = Function::True;
            break;
        }
        case Function::Cdr:
        {
            value->setValueType(ValueType::Apply);
            value->m_applyData.m_funcValue = argValue;
            value->m_applyData.m_argValue.init(ValueType::Closure);
            value->m_applyData.m_argValue->m_closureData.m_func = Function::False;
            break;
        }
        case Function::Nil:
        {
            value->setValueType(ValueType::Closure);
            value->m_closureData.m_func = Function::True;
            break;
        }
        case Function::IsNil:
        {
            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType == ValueType::Closure)
            {
                if (argValue->m_closureData.m_func == Function::Nil &&
                    argValue->m_closureData.m_size == 0)
                {
                    value->setValueType(ValueType::Closure);
                    value->m_closureData.m_func = Function::True;
                    return true;
                }
                if (argValue->m_closureData.m_func == Function::Cons &&
                    argValue->m_closureData.m_size == 2)
                {
                    value->setValueType(ValueType::Closure);
                    value->m_closureData.m_func = Function::False;
                    return true;
                }
            }

            if (pMsg) *pMsg = "Bad argument to isnil";
            return false;
        }
        //case Function::Vec:
        case Function::Draw:
        {
            vector<pair<int64_t, int64_t>> pts;
            Value curValue = argValue;
            while (true)
            {
                if (!eval(curValue, pMsg)) return false;
                if (curValue->m_valueType == ValueType::Closure &&
                    curValue->m_closureData.m_func == Function::Nil &&
                    curValue->m_closureData.m_size == 0)
                {
                    break;
                }
                if (curValue->m_valueType == ValueType::Closure &&
                    curValue->m_closureData.m_func == Function::Cons &&
                    curValue->m_closureData.m_size == 2)
                {
                    auto& ptValue = curValue->m_closureData.m_args[0];
                    if (!eval(ptValue, pMsg)) return false;
                    if (ptValue->m_valueType == ValueType::Closure &&
                        ptValue->m_closureData.m_func == Function::Cons &&
                        ptValue->m_closureData.m_size == 2)
                    {
                        auto& ptArgs = ptValue->m_closureData.m_args;
                        if (!eval(ptArgs[0], pMsg)) return false;
                        if (!eval(ptArgs[1], pMsg)) return false;
                        if (ptArgs[0]->m_valueType != ValueType::Integer ||
                            ptArgs[1]->m_valueType != ValueType::Integer)
                        {
                            if (pMsg) *pMsg = "Bad argument to draw";
                            return false;
                        }
                        int64_t x = 0;
                        int64_t y = 0;
                        if (!Int::getValue(ptArgs[0]->m_integerData.m_value,
                                           &x,
                                           pMsg))
                        {
                            return false;
                        }
                        if (!Int::getValue(ptArgs[1]->m_integerData.m_value,
                                           &y,
                                           pMsg))
                        {
                            return false;
                        }
                        pts.emplace_back(x, y);
                    }
                    else
                    {
                        if (pMsg) *pMsg = "Bad argument to draw";
                        return false;
                    }
                    curValue = curValue->m_closureData.m_args[1];
                    continue;
                }
                if (pMsg) *pMsg = "Bad argument to draw";
                return false;
            }
            int64_t sizeX = 0;
            int64_t sizeY = 0;
            for (auto& pt : pts)
            {
                if (pt.first < 0 || pt.second < 0)
                {
                    if (pMsg) *pMsg = "Negative coordinate in draw";
                    return false;
                }
                if (pt.first >= 1024 || pt.second >= 1024)
                {
                    if (pMsg) *pMsg = "Large coordinate in draw";
                    return false;
                }
                sizeX = std::max(sizeX, pt.first + 1);
                sizeY = std::max(sizeY, pt.second + 1);
            }
            value->setValueType(ValueType::Picture);
            auto& picture = value->m_pictureData.m_picture;
            picture.resize(sizeX, sizeY);
            for (auto& pt : pts)
            {
                picture(pt.first, pt.second) = 1;
            }
            break;
        }
        //case Function::Checkerboard:
        case Function::MultipleDraw:
        {
            if (!eval(argValue, pMsg)) return false;
            if (argValue->m_valueType == ValueType::Closure &&
                argValue->m_closureData.m_func == Function::Nil &&
                argValue->m_closureData.m_size == 0)
            {
                value->setValueType(ValueType::Closure);
                value->m_closureData.m_func = Function::Nil;
                return true;
            }
            if (argValue->m_valueType == ValueType::Closure &&
                argValue->m_closureData.m_func == Function::Cons &&
                argValue->m_closureData.m_size == 2)
            {
                value->setValueType(ValueType::Closure);
                value->m_closureData.m_func = Function::Cons;
                value->m_closureData.m_size = 2;
                value->m_closureData.m_args[0].init(ValueType::Apply);
                value->m_closureData.m_args[0]->m_applyData.m_funcValue.init(ValueType::Closure);
                value->m_closureData.m_args[0]->m_applyData.m_funcValue->m_closureData.m_func = Function::Draw;
                value->m_closureData.m_args[0]->m_applyData.m_argValue = argValue->m_closureData.m_args[0];
                value->m_closureData.m_args[1].init(ValueType::Apply);
                value->m_closureData.m_args[1]->m_applyData.m_funcValue.init(ValueType::Closure);
                value->m_closureData.m_args[1]->m_applyData.m_funcValue->m_closureData.m_func = Function::MultipleDraw;
                value->m_closureData.m_args[1]->m_applyData.m_argValue = argValue->m_closureData.m_args[1];
                return true;
            }
            if (pMsg) *pMsg = "Bad argument to multipledraw";
            return false;
        }
        case Function::If0:
        {
            if (!eval(args[0], pMsg)) return false;
            if (args[0]->m_valueType == ValueType::Integer)
            {
                if (Int::eq(args[0]->m_integerData.m_value, Int(0)))
                {
                    value = args[1];
                    break;
                }
                if (Int::eq(args[0]->m_integerData.m_value, Int(1)))
                {
                    value = argValue;
                    break;
                }
                if (pMsg) *pMsg = "Bad integer argument to if0";
                return false;
            }

            if (pMsg) *pMsg = "Bad argument to if0";
            return false;
        }
        default:
        {
            if (pMsg) *pMsg = "Unexpected function in closure";
            return false;
        }
        }
    }

    return true;
}
