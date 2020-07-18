#include "PrintValue.hpp"
#include "Value.hpp"
#include "StringUtils.hpp"
#include "Eval.hpp"

using std::string;

namespace
{
    typedef std::function<void(const string&)> OutFn;

    bool printValueImpl(const OutFn& outFn,
                        Value& value,
                        bool force,
                        string* pMsg)
    {
        if (!value)
        {
            outFn("<null>");
            return true;
        }

        if (force)
        {
            if (!eval(value, pMsg))
            {
                return false;
            }
        }

        switch (value->m_valueType)
        {
        case ValueType::Invalid:
        {
            outFn("<invalid>");
            return true;
        }
        case ValueType::Apply:
        {
            outFn("<apply>");
            return true;
        }
        case ValueType::Integer:
        {
            outFn(Int::format(value->m_integerData.m_value));
            return true;
        }
        case ValueType::Closure:
        {
            Function func = value->m_closureData.m_func;
            uint32_t size = value->m_closureData.m_size;
            auto& args = value->m_closureData.m_args;
            string name;
            switch (func)
            {
            case Function::Inc: name = "inc"; break;
            case Function::Dec: name = "dec"; break;
            case Function::Add: name = "add"; break;
            case Function::Mul: name = "mul"; break;
            case Function::Div: name = "div"; break;
            case Function::Eq: name = "eq"; break;
            case Function::Lt: name = "lt"; break;
            case Function::Modulate: name = "mod"; break;
            case Function::Demodulate: name = "dem"; break;
            case Function::Send: name = "send"; break;
            case Function::Neg: name = "neg"; break;
            case Function::S: name = "s"; break;
            case Function::C: name = "c"; break;
            case Function::B: name = "b"; break;
            case Function::True: name = "t"; break;
            case Function::False: name = "f"; break;
            case Function::I: name = "i"; break;
            case Function::Cons: name = "cons"; break;
            case Function::Car: name = "car"; break;
            case Function::Cdr: name = "cdr"; break;
            case Function::Nil: name = "nil"; break;
            case Function::IsNil: name = "isnil"; break;
            case Function::Vec: name = "vec"; break;
            case Function::Draw: name = "draw"; break;
            case Function::Checkerboard: name = "chkb"; break;
            case Function::MultipleDraw: name = "multipledraw"; break;
            case Function::If0: name = "if0"; break;
            default: name = strprintf("func%" PRIu32 "", (uint32_t)func);
            }

            outFn("<" + name);
            for (uint32_t i = 0; i < size; i++)
            {
                outFn(" ");
                if (!printValueImpl(outFn,
                                    args[i],
                                    force,
                                    pMsg))
                {
                    return false;
                }
            }
            outFn(">");

            return true;
        }
        case ValueType::Signal:
        {
            outFn('"' + value->m_signalData.m_signal + '"');
            return true;
        }
        case ValueType::Picture:
        {
            auto& picture = value->m_pictureData.m_picture;
            size_t width = picture.getWidth();
            size_t height = picture.getHeight();
            size_t w = std::max(width + 3, (size_t)17);
            size_t h = std::max(height + 3, (size_t)13);
            string str;
            str += '\n';
            str += ' ';
            for (size_t x = 0; x < w; x++) str += '-';
            str += ' ';
            str += '\n';
            for (size_t y = 0; y < h; y++)
            {
                str += '|';
                for (size_t x = 0; x < w; x++)
                {
                    if (x < width && y < height && picture(x, y))
                    {
                        str += '*';
                    }
                    else
                    {
                        str += ' ';
                    }
                }
                str += '|';
            str += '\n';
            }
            str += ' ';
            for (size_t x = 0; x < w; x++) str += '-';
            str += ' ';
            str += '\n';
            outFn(str);
            return true;
        }
        default:
        {
            outFn("<unknown>");
            return true;
        }
        }
    }
}

bool printValue(string* pStr,
                Value& value,
                bool force,
                string* pMsg)
{
    string str;
    auto outFn = [&](const string& substr)
    {
        str += substr;
    };
    if (!printValueImpl(outFn, value, force, pMsg))
    {
        return false;
    }

    if (pStr) *pStr = std::move(str);
    return true;
}

bool printValue(Value& value,
                bool force,
                string* pMsg)
{
    auto outFn = [](const string& substr)
    {
        printf("%s", substr.c_str());
        fflush(stdout);
    };
    if (!printValueImpl(outFn, value, force, pMsg))
    {
        return false;
    }

    return true;
}
