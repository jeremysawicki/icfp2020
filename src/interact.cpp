#include "Common.hpp"
#include "Protocol.hpp"
#include "Cleanup.hpp"
#include "FileUtils.hpp"
#include "Token.hpp"
#include "TokenText.hpp"
#include "SymTable.hpp"
#include "ParseValue.hpp"
#include "Bindings.hpp"
#include "Value.hpp"
#include "Eval.hpp"
#include "Modem.hpp"
#include "Graphics.hpp"
#include "TimeUtils.hpp"
#include "PrintValue.hpp"
#include "FormatValue.hpp"
#include "Protocol.hpp"

using std::string;
using std::vector;
using std::pair;

bool parsePoints(Value value,
                 vector<pair<int32_t, int32_t>>& pts,
                 string* pMsg)
{
    pts.clear();
    while (true)
    {
        if (!eval(value, pMsg)) return false;
        if (value->m_valueType == ValueType::Closure &&
            value->m_closureData.m_func == Function::Nil &&
            value->m_closureData.m_size == 0)
        {
            break;
        }
        if (value->m_valueType == ValueType::Closure &&
            value->m_closureData.m_func == Function::Cons &&
            value->m_closureData.m_size == 2)
        {
            auto& ptValue = value->m_closureData.m_args[0];
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
                    if (pMsg) *pMsg = "Bad point data";
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
                if (x < std::numeric_limits<int32_t>::min() ||
                    x > std::numeric_limits<int32_t>::max() ||
                    y < std::numeric_limits<int32_t>::min() ||
                    y > std::numeric_limits<int32_t>::max())
                {
                    if (pMsg) *pMsg = "Coordinate does not fit in 32 bits";
                    return false;
                }
                pts.emplace_back((int32_t)x, (int32_t)y);
            }
            else
            {
                if (pMsg) *pMsg = "Bad point data";
                return false;
            }
            value = value->m_closureData.m_args[1];
            continue;
        }
        if (pMsg) *pMsg = "Bad point data";
        return false;
    }

    return true;
}

bool parsePictures(Value value,
                   vector<vector<pair<int32_t, int32_t>>>& pics,
                   string* pMsg)
{
    pics.clear();
    while (true)
    {
        if (!eval(value, pMsg)) return false;
        if (value->m_valueType == ValueType::Closure &&
            value->m_closureData.m_func == Function::Nil &&
            value->m_closureData.m_size == 0)
        {
            break;
        }
        if (value->m_valueType == ValueType::Closure &&
            value->m_closureData.m_func == Function::Cons &&
            value->m_closureData.m_size == 2)
        {
            auto& picValue = value->m_closureData.m_args[0];
            auto& pic = pics.emplace_back();
            if (!parsePoints(picValue, pic, pMsg)) return false;
            value = value->m_closureData.m_args[1];
            continue;
        }
        if (pMsg) *pMsg = "Bad picture data";
        return false;
    }

    return true;
}

void usage(FILE* f)
{
    fprintf(f, "Usage: interact [<options>] <file> <protocol> [<state>]\n");
    fprintf(f, "  <file>\n");
    fprintf(f, "        Bindings file containing protocol definition\n");
    fprintf(f, "  <protocol>\n");
    fprintf(f, "        Protocol name\n");
    fprintf(f, "  <state>\n");
    fprintf(f, "        Initial protocol state (default: nil)\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -h    Print usage information and exit\n");
}

int main(int argc, char *argv[])
{
    bool gotFileName = false;
    bool gotProtocolName = false;
    bool gotStateText = false;

    bool help = false;
    string fileName;
    string protocolName;
    string stateText;

    int iArg = 1;
    while (iArg < argc)
    {
        string strArg = argv[iArg++];

        if (strArg == "-h" || strArg == "--help")
        {
            help = true;
        }
        else if (!gotFileName)
        {
            fileName = strArg;
            gotFileName = true;
        }
        else if (!gotProtocolName)
        {
            protocolName = strArg;
            gotProtocolName = true;
        }
        else if (!gotStateText)
        {
            stateText = strArg;
            gotStateText = true;
        }
        else
        {
            usage(stderr);
            return 1;
        }
    }

    if (help)
    {
        usage(stdout);
        return 0;
    }

    if (!gotFileName ||
        !gotProtocolName)
    {
        usage(stderr);
        return 1;
    }

    string msg;

    Protocol::init();
    Cleanup cleanupProtocol([](){ Protocol::cleanup(); });

    if (!Protocol::initAPIKey(false, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    string text;
    if (!readFile(fileName, &text))
    {
        fprintf(stderr, "Error reading file\n");
        return 1;
    }

    SymTable symTable;
    vector<Token> tokens;
    if (!parseTokenText(symTable, text, &tokens, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    Bindings bindings;
    bindings.resize(symTable.size());
    if (!parseBindings(tokens, bindings, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    uint32_t protocolId = 0;
    if (!symTable.getId(protocolName, &protocolId))
    {
        fprintf(stderr, "Protocol symbol not found\n");
        return 1;
    }

    if (protocolId >= bindings.m_values.size() ||
        !bindings.m_values[protocolId] ||
        bindings.m_values[protocolId]->getValueType() == ValueType::Invalid)
    {
        fprintf(stderr, "Protocol binding not found\n");
        return 1;
    }

    Value protocol = bindings.m_values[protocolId];

    Value state;
    if (gotStateText)
    {
        if (!parseValueText(symTable, stateText, bindings, state, &msg))
        {
            fprintf(stderr, "Error parsing initial state\n");
            fprintf(stderr, "%s\n", msg.c_str());
            return 1;
        }
    }
    else
    {
        state.init(ValueType::Closure);
        state->m_closureData.m_func = Function::Nil;
    }

    vector<vector<pair<int32_t, int32_t>>> pics;

    auto interact = [&](int32_t x, int32_t y, string* pMsg) -> bool
    {
        Value data;
        data.init(ValueType::Closure);
        data->m_closureData.m_func = Function::Cons;
        data->m_closureData.m_size = 2;
        data->m_closureData.m_args[0].init(ValueType::Integer);
        data->m_closureData.m_args[0]->m_integerData.m_value = Int(x);
        data->m_closureData.m_args[1].init(ValueType::Integer);
        data->m_closureData.m_args[1]->m_integerData.m_value = Int(y);

        while (true)
        {
            Value cons1;
            cons1.init(ValueType::Apply);
            cons1->m_applyData.m_funcValue.init(ValueType::Apply);
            cons1->m_applyData.m_funcValue->m_applyData.m_funcValue = protocol;
            cons1->m_applyData.m_funcValue->m_applyData.m_argValue = state;
            cons1->m_applyData.m_argValue = data;

            if (!eval(cons1, pMsg)) return false;

            if (cons1->m_valueType != ValueType::Closure ||
                cons1->m_closureData.m_func != Function::Cons ||
                cons1->m_closureData.m_size != 2)
            {
                if (pMsg) *pMsg = "Invalid result";
                return false;
            }

            Value elem1 = cons1->m_closureData.m_args[0];
            Value cons2 = cons1->m_closureData.m_args[1];
            if (!eval(elem1, pMsg)) return false;
            if (!eval(cons2, pMsg)) return false;

            if (elem1->m_valueType != ValueType::Integer ||
                cons2->m_valueType != ValueType::Closure ||
                cons2->m_closureData.m_func != Function::Cons ||
                cons2->m_closureData.m_size != 2)
            {
                if (pMsg) *pMsg = "Invalid result";
                return false;
            }

            Int flag = elem1->m_integerData.m_value;
            Value elem2 = cons2->m_closureData.m_args[0];
            Value cons3 = cons2->m_closureData.m_args[1];
            if (!eval(cons3, pMsg)) return false;

            if (cons3->m_valueType != ValueType::Closure ||
                cons3->m_closureData.m_func != Function::Cons ||
                cons3->m_closureData.m_size != 2)
            {
                if (pMsg) *pMsg = "Invalid result";
                return false;
            }

            Value elem3 = cons3->m_closureData.m_args[0];
            Value cons4 = cons3->m_closureData.m_args[1];

            if (cons4->m_valueType != ValueType::Closure ||
                cons4->m_closureData.m_func != Function::Nil ||
                cons4->m_closureData.m_size != 0)
            {
                if (pMsg) *pMsg = "Invalid result";
                return false;
            }

#if 0
            if (!printValue(elem2, true, &msg))
            {
                printf("\n");
                printf("%s\n", msg.c_str());
                return 1;
            }
            printf("\n");
#endif

#if 1
            string oldStateText;
            if (!formatValueText(symTable, state, &oldStateText, pMsg))
            {
                return false;
            }
            printf("> %s\n", oldStateText.c_str());

            string newStateText;
            if (!formatValueText(symTable, elem2, &newStateText, pMsg))
            {
                return false;
            }
            printf("< %s\n", newStateText.c_str());
            printf("\n");
#endif

            string stateSignal;
            if (!modulate(elem2, stateSignal, pMsg)) return false;
            Value newState;
            newState.init();
            //printf("stateSignal = '%s'\n", stateSignal.c_str());
            if (!demodulate(stateSignal, newState, pMsg)) return false;
            state = std::move(newState);

            if (Int::eq(flag, Int(0)))
            {
                if (!parsePictures(elem3, pics, pMsg)) return false;
                break;
            }
            else if (Int::eq(flag, Int(1)))
            {
#if 0
                if (pMsg) *pMsg = "send not implemented";
                return false;
#endif
#if 1
                string requestText;
                if (!formatValueText(symTable, elem3, &requestText, pMsg))
                {
                    return false;
                }
                printf("> %s\n", requestText.c_str());
                string request;
                if (!modulate(elem3, request, pMsg)) return false;
                printf("> \"%s\"\n", request.c_str());
                //sleepMS(500);
                string response;
                if (!Protocol::send(request, &response, pMsg)) return false;
                printf("< \"%s\"\n", response.c_str());
                if (!demodulate(response, data, pMsg)) return false;
                string responseText;
                if (!formatValueText(symTable, data, &responseText, pMsg))
                {
                    return false;
                }
                printf("< %s\n", responseText.c_str());
                printf("\n");
#endif
            }
            else
            {
                if (pMsg) *pMsg = "Invalid flag value";
                return false;
            }
        }

        return true;
    };

    if (!interact(0, 0, &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    bool done = false;

    uint32_t zoom = 3;

    Graphics gr;

    auto onDestroy = [&]()
    {
        done = true;
    };
    gr.setOnDestroy(onDestroy);

    auto onPaint = [&]()
    {
        gr.beginPaint(true);

        uint32_t w = gr.getWidth();
        uint32_t h = gr.getHeight();

        gr.fillRect(0, 0, w, h, 0x000000);

        w = (w + zoom - 1) / zoom;
        h = (h + zoom - 1) / zoom;
        int32_t offsetX = (int32_t)w / 2;
        int32_t offsetY = (int32_t)h / 2;

        size_t numPics = pics.size();
        vector<uint32_t> colors(numPics);
        uint32_t component = 0xff;
        for (size_t iPic = 0; iPic < numPics; iPic++)
        {
            uint32_t color = component * 0x010101;
            colors[iPic] = color;
            component = component * 192 / 256;
        }
        for (size_t iPic = numPics; iPic > 0; )
        {
            iPic--;
            auto& pic = pics[iPic];
            uint32_t color = colors[iPic];
            for (auto& pt : pic)
            {
                int32_t x = pt.first + offsetX;
                int32_t y = pt.second + offsetY;
                if (x >= 0 && x < (int32_t)w && y >= 0 && y < (int32_t)h)
                {
                    gr.fillRect(x * zoom, y * zoom, zoom, zoom, color);
                }
            }
        }

        gr.endPaint();
    };
    gr.setOnPaint(onPaint);

    auto onKey = [&](Key key)
    {
        if (key == Key::Q)
        {
            gr.destroy();
        }
    };
    gr.setOnKey(onKey);

    auto onClick = [&](uint32_t x, uint32_t y)
    {
        uint32_t w = gr.getWidth();
        uint32_t h = gr.getHeight();
        w = (w + zoom - 1) / zoom;
        h = (h + zoom - 1) / zoom;
        int32_t offsetX = (int32_t)w / 2;
        int32_t offsetY = (int32_t)h / 2;
        int32_t xx = (int32_t)(x / zoom) - offsetX;
        int32_t yy = (int32_t)(y / zoom) - offsetY;

        printf("(%" PRIi32 ", %" PRIi32 ") -> (%" PRIi32 ", %" PRIi32 ")\n", x, y, xx, yy);
        if (!interact(xx, yy, &msg))
        {
            fprintf(stderr, "%s\n", msg.c_str());
            gr.destroy();
            return;
        }
        gr.requestPaint();
    };
    gr.setOnClick(onClick);

    if (!gr.create(1400, 900, "interact", &msg))
    {
        fprintf(stderr, "%s\n", msg.c_str());
        return 1;
    }

    /*
    while (true)
    {
        while (Graphics::update(false))
        {
            if (done)
            {
                break;
            }
        }

        if (done)
        {
            break;
        }

        sleepMS(10);
    }
    */

    while (Graphics::update(true))
    {
        if (done)
        {
            break;
        }
    }

    return 0;
}
