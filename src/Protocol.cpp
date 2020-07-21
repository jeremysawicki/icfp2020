#include "Protocol.hpp"
#include "Cleanup.hpp"
#include "StringUtils.hpp"
#include "FileUtils.hpp"
#include "Value.hpp"
#include "Modem.hpp"
#include "SymTable.hpp"
#include "FormatValue.hpp"
#include <curl/curl.h>

// sudo apt-get install libcurl4-openssl-dev

using std::string;
using std::vector;

namespace Protocol
{
    void init()
    {
        CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_global_init failed\n");
            exit(1);
        }
    }

    void cleanup()
    {
        curl_global_cleanup();
    }

    string urlPrefix;
    string urlSuffix;
    bool verbose = false;

    bool getAPIKey(string* pKey)
    {
        string tmpKey;
        if (!readFile("../../../apikey.txt", &tmpKey))
        {
            return false;
        }

        string key;
        for (char ch : tmpKey)
        {
            if (ch >= 33 && ch <= 126)
            {
                key.push_back(ch);
            }
        }
        if (key.empty())
        {
            return false;
        }

        if (pKey) *pKey = std::move(key);
        return true;
    }

    bool initAPIKey(bool verbose_,
                    string* pMsg)
    {
        string key;
        if (!getAPIKey(&key))
        {
            if (pMsg) *pMsg = "Error reading API key";
            return false;
        }

        urlPrefix = "https://icfpc2020-api.testkontur.ru";
        urlSuffix = "?apiKey=" + key;
        verbose = verbose_;
        return true;
    }

    bool initDocker(const string& url,
                    bool verbose_,
                    string* pMsg)
    {
        urlPrefix = url;
        urlSuffix = "";
        verbose = verbose_;
        return true;
    }

    size_t writeFunction(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        string* pStr = (string*)userdata;
        pStr->append(ptr, size * nmemb);
        return size * nmemb;
    }

    bool makeRequest(const string& path,
                     const string& request,
                     string* pResponse,
                     string* pMsg)
    {
        string strResponse;

        CURL* curl = curl_easy_init();
        if (!curl)
        {
            if (pMsg) *pMsg = "curl_easy_init failed";
            return false;
        }
        Cleanup cleanupCurl([&](){ curl_easy_cleanup(curl); });

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: text/plain");
        Cleanup cleanupHeaders([&](){ curl_slist_free_all(headers); });

        char errorBuf[CURL_ERROR_SIZE];

        string url = urlPrefix + path + urlSuffix;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuf);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, request.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        errorBuf[0] = 0;

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            if (pMsg)
            {
                *pMsg = "curl request failed";
                size_t errorLen = strlen(errorBuf);
                if (errorLen > 0)
                {
                    if (errorBuf[errorLen - 1] == '\n')
                    {
                        errorBuf[errorLen - 1] = 0;
                        errorLen--;
                    }
                }
                if (errorLen > 0)
                {
                    *pMsg += ": ";
                    *pMsg += errorBuf;
                }
            }
            return false;
        }

        long code = 0;
        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (res != CURLE_OK)
        {
            if (pMsg) *pMsg = "curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed";
            return false;
        }

        //printf("%s\n", strResponse.c_str());

        if (code != 200)
        {
            printf("%s\n", strResponse.c_str());
            if (pMsg) *pMsg = strprintf("curl response code %lu", code);
            return false;
        }

        if (pResponse)
        {
            *pResponse = std::move(strResponse);
        }

        return true;
    }

    bool makeRequest(const string& path,
                     Value& request,
                     Value* pResponse,
                     string* pMsg)
    {
        if (verbose)
        {
            SymTable symTable;
            string requestText;
            if (!formatValueText(symTable, request, &requestText, pMsg))
            {
                return false;
            }
            printf("> %s\n", requestText.c_str());
        }

        string requestSignal;
        if (!modulate(request, requestSignal, pMsg))
        {
            return false;
        }

        string responseSignal;
        if (!makeRequest(path, requestSignal, &responseSignal, pMsg))
        {
            return false;
        }

        pResponse->init();
        if (!demodulate(responseSignal, *pResponse, pMsg))
        {
            return false;
        }

        if (verbose)
        {
            SymTable symTable;
            string responseText;
            if (!formatValueText(symTable, *pResponse, &responseText, pMsg))
            {
                return false;
            }
            printf("< %s\n", responseText.c_str());
            printf("\n");
        }

        return true;
    }

    Value makeInt(int64_t intValue)
    {
        Value value;
        value.init(ValueType::Integer);
        value->m_integerData.m_value = Int(intValue);
        return value;
    }

    Value makeNil()
    {
        Value value;
        value.init(ValueType::Closure);
        value->m_closureData.m_func = Function::Nil;
        return value;
    }

    Value makeCons(Value car, Value cdr)
    {
        Value value;
        value.init(ValueType::Closure);
        value->m_closureData.m_func = Function::Cons;
        value->m_closureData.m_size = 2;
        value->m_closureData.m_args[0] = car;
        value->m_closureData.m_args[1] = cdr;
        return value;
    }

    Value makeList()
    {
        return makeNil();
    }

    template<typename... Args>
    Value makeList(Value head, Args... tail)
    {
        return makeCons(head, makeList(tail...));
    }

    Value makeList(const std::vector<Value>& list)
    {
        Value value = makeNil();
        for (size_t i = list.size(); i > 0; )
        {
            i--;
            value = makeCons(list[i], value);
        }
        return value;
    }

    bool isInt(const Value& value)
    {
        return value &&
            value->getValueType() == ValueType::Integer;
    }

    bool isNil(const Value& value)
    {
        return value &&
            value->getValueType() == ValueType::Closure &&
            value->m_closureData.m_func == Function::Nil &&
            value->m_closureData.m_size == 0;
    }

    bool isCons(const Value& value)
    {
        return value &&
            value->getValueType() == ValueType::Closure &&
            value->m_closureData.m_func == Function::Cons &&
            value->m_closureData.m_size == 2;
    }

    int64_t getInt(const Value& value)
    {
        int64_t intValue = 0;
        Int::getValue(value->m_integerData.m_value, &intValue, nullptr);
        return intValue;
    }

    Value getCar(const Value& value)
    {
        return value->m_closureData.m_args[0];
    }

    Value getCdr(const Value& value)
    {
        return value->m_closureData.m_args[1];
    }

    bool isList(Value value)
    {
        while (isCons(value))
        {
            value = getCdr(value);
        }
        return isNil(value);
    }

    vector<Value> getList(Value value)
    {
        vector<Value> list;
        while (isCons(value))
        {
            list.push_back(getCar(value));
            value = getCdr(value);
        }
        return list;
    }

    bool test(const string& playerKey,
              string* pResponse,
              string* pMsg)
    {
        if (!makeRequest("/",
                         playerKey,
                         pResponse,
                         pMsg))
        {
            return false;
        }

        return true;
    }

    bool send(const string& request,
              string* pResponse,
              string* pMsg)
    {
        if (!makeRequest("/aliens/send",
                         request,
                         pResponse,
                         pMsg))
        {
            return false;
        }

        return true;
    }

    bool isSuccess(Value& response)
    {
        if (!isList(response))
        {
            return false;
        }
        vector<Value> list1 = getList(response);
        if (list1.size() < 1 ||
            !isInt(list1[0]) ||
            getInt(list1[0]) != 1)
        {
            return false;
        }

        return true;
    }

    bool parseRole(Value& value,
                   Role* pRole)
    {
        if (!isInt(value))
        {
            return false;
        }
        int64_t iRole = getInt(value);
        if (iRole != 0 && iRole != 1)
        {
            return false;
        }
        if (pRole) *pRole = (Role)iRole;
        return true;
    }

    bool parseInfo(Value& response,
                   Info* pInfo)
    {
        if (!isList(response))
        {
            return false;
        }
        vector<Value> list1 = getList(response);
        if (list1.size() < 2 ||
            !isInt(list1[1]))
        {
            return false;
        }

        int64_t iStage = getInt(list1[1]);
        if (iStage != 0 && iStage != 1 && iStage != 2)
        {
            return false;
        }
        pInfo->m_stage = (Stage)iStage;

        if (iStage == 2)
        {
            return true;
        }

        if (list1.size() < 3 ||
            !isList(list1[2]))
        {
            return false;
        }

        vector<Value> list2 = getList(list1[2]);
        if (list2.size() < 4 ||
            !isInt(list2[0]) ||
            !isList(list2[2]) ||
            !isList(list2[3]))
        {
            return false;
        }
        pInfo->m_maxTicks = getInt(list2[0]);
        if (!parseRole(list2[1], &pInfo->m_role))
        {
            return false;
        }
        vector<Value> list3 = getList(list2[2]);
        if (list3.size() < 3 ||
            !isInt(list3[0]) ||
            !isInt(list3[1]) ||
            !isInt(list3[2]))
        {
            return false;
        }
        pInfo->m_maxCost = getInt(list3[0]);
        pInfo->m_maxAccel = getInt(list3[1]);
        pInfo->m_maxHeat = getInt(list3[2]);
        vector<Value> list4 = getList(list2[3]);
        if (list4.size() == 0)
        {
            pInfo->m_minRadius = -1;
            pInfo->m_maxRadius = -1;
        }
        else if (list4.size() == 2)
        {
            if (!isInt(list4[0]) ||
                !isInt(list4[1]))
            {
                return false;
            }
            pInfo->m_minRadius = getInt(list4[0]);
            pInfo->m_maxRadius = getInt(list4[1]);
        }
        else
        {
            return false;
        }
        return true;
    }

    bool parseVec(Value& value,
                  Vec* pVec)
    {
        if (!isCons(value))
        {
            return false;
        }
        Value car = getCar(value);
        Value cdr = getCdr(value);
        if (!isInt(car) ||
            !isInt(cdr))
        {
            return false;
        }
        pVec->m_x = getInt(car);
        pVec->m_y = getInt(cdr);
        return true;
    }

    Value formatVec(const Vec& vec)
    {
        return makeCons(makeInt(vec.m_x), makeInt(vec.m_y));
    }

    bool parseParams(Value& value,
                     Params* pParams)
    {
        if (!isList(value))
        {
            return false;
        }
        vector<Value> list1 = getList(value);
        if (list1.size() < 4 ||
            !isInt(list1[0]) ||
            !isInt(list1[1]) ||
            !isInt(list1[2]) ||
            !isInt(list1[3]))
        {
            return false;
        }
        pParams->m_fuel = getInt(list1[0]);
        pParams->m_guns = getInt(list1[1]);
        pParams->m_cooling = getInt(list1[2]);
        pParams->m_ships = getInt(list1[3]);
        return true;
    }

    bool parseCommandType(Value& value,
                          CommandType* pCommandType)
    {
        if (!isInt(value))
        {
            return false;
        }
        int64_t iCommandType = getInt(value);
        if (iCommandType != 0 &&
            iCommandType != 1 &&
            iCommandType != 2 &&
            iCommandType != 3)
        {
            // To be safe
            //return false;
        }
        *pCommandType = (CommandType)iCommandType;
        return true;
    }

    Value formatCommand(const Command& command)
    {
        if (command.m_commandType == CommandType::Accelerate)
        {
            return makeList(makeInt(0),
                            makeInt(command.m_id),
                            formatVec(command.m_vec));
        }
        if (command.m_commandType == CommandType::Detonate)
        {
            return makeList(makeInt(1),
                            makeInt(command.m_id));
        }
        if (command.m_commandType == CommandType::Shoot)
        {
            return makeList(makeInt(2),
                            makeInt(command.m_id),
                            formatVec(command.m_vec),
                            makeInt(command.m_val));
        }
        if (command.m_commandType == CommandType::Clone)
        {
            return makeList(makeInt(3),
                            makeInt(command.m_id),
                            makeList(makeInt(command.m_params.m_fuel),
                                     makeInt(command.m_params.m_guns),
                                     makeInt(command.m_params.m_cooling),
                                     makeInt(command.m_params.m_ships)));
        }

        return makeNil();
    }

    Value formatCommands(const vector<Command>& commands)
    {
        size_t numCommands = commands.size();
        vector<Value> list(numCommands);
        for (size_t iCommand = 0; iCommand < numCommands; iCommand++)
        {
            list[iCommand] = formatCommand(commands[iCommand]);
        }
        return makeList(list);
    }

    bool parseEffect(Value& value,
                     Effect* pEffect)
    {
        if (!isList(value))
        {
            return false;
        }
        vector<Value> list1 = getList(value);
        if (list1.size() < 1)
        {
            return false;
        }
        if (!parseCommandType(list1[0], &pEffect->m_commandType))
        {
            return false;
        }
        return true;
    }

    bool parseShip(Value& value,
                   Ship* pShip)
    {
        if (!isList(value))
        {
            return false;
        }
        vector<Value> list1 = getList(value);
        if (list1.size() < 2 ||
            !isList(list1[0]) ||
            !isList(list1[1]))
        {
            return false;
        }
        vector<Value> list2 = getList(list1[0]);
        if (list2.size() < 8 ||
            !isInt(list2[1]) ||
            !isInt(list2[5]) ||
            !isInt(list2[6]) ||
            !isInt(list2[7]))
        {
            return false;
        }
        if (!parseRole(list2[0], &pShip->m_role))
        {
            return false;
        }
        pShip->m_id = getInt(list2[1]);
        pShip->m_heat = getInt(list2[5]);
        pShip->m_maxHeat = getInt(list2[6]);
        pShip->m_maxAccel = getInt(list2[7]);
        if (!parseVec(list2[2], &pShip->m_pos)) return false;
        if (!parseVec(list2[3], &pShip->m_vel)) return false;
        if (!parseParams(list2[4], &pShip->m_params)) return false;
        vector<Value> list3 = getList(list1[1]);
        size_t numEffects = list3.size();
        pShip->m_effects.resize(numEffects);
        for (size_t iEffect = 0; iEffect < numEffects; iEffect++)
        {
            if (!parseEffect(list3[iEffect], &pShip->m_effects[iEffect]))
            {
                return false;
            }
        }
        return true;
    }

    bool parseState(Value& response,
                    State* pState)
    {
        if (!isList(response))
        {
            return false;
        }
        vector<Value> list1 = getList(response);
        if (list1.size() < 4 ||
            !isList(list1[3]))
        {
            return false;
        }

        vector<Value> list2 = getList(list1[3]);
        if (list2.size() < 3 ||
            !isInt(list2[0]) ||
            !isList(list2[2]))
        {
            return false;
        }
        pState->m_tick = getInt(list2[0]);

        vector<Value> list3 = getList(list2[2]);
        size_t numShips = list3.size();
        pState->m_ships.resize(numShips);
        for (size_t iShip = 0; iShip < numShips; iShip++)
        {
            if (!parseShip(list3[iShip], &pState->m_ships[iShip]))
            {
                return false;
            }
        }
        return true;
    }

    bool createTutorial(int64_t tutorialNum,
                        Role* pRole,
                        int64_t* pPlayerKey,
                        string* pMsg)
    {
        Value request = makeList(makeInt(1), makeInt(tutorialNum));
        Value response;
        if (!makeRequest("/aliens/send", request, &response, pMsg))
        {
            return false;
        }

        if (!isList(response))
        {
            if (pMsg) *pMsg = "createTutorial: invalid response 1";
            return false;
        }
        vector<Value> list1 = getList(response);
        if (list1.size() < 1 ||
            !isInt(list1[0]))
        {
            if (pMsg) *pMsg = "createTutorial: invalid response 2";
            return false;
        }
        if (getInt(list1[0]) != 1)
        {
            if (pMsg) *pMsg = "createTutorial: protocol error";
            return false;
        }
        if (list1.size() < 2 ||
            !isList(list1[1]))
        {
            if (pMsg) *pMsg = "createTutorial: invalid response 3";
            return false;
        }
        vector<Value> list2 = getList(list1[1]);
        if (list2.size() != 1 ||
            !isList(list2[0]))
        {
            if (pMsg) *pMsg = "createTutorial: invalid response 4";
            return false;
        }
        vector<Value> list3 = getList(list2[0]);
        if (list3.size() != 2 ||
            !isInt(list3[0]) ||
            !isInt(list3[1]))
        {
            if (pMsg) *pMsg = "createTutorial: invalid response 5";
            return false;
        }
        if (!parseRole(list3[0], pRole))
        {
            if (pMsg) *pMsg = "createTutorial: invalid response 6";
            return false;
        }
        if (pPlayerKey) *pPlayerKey = getInt(list3[1]);

        return true;
    }

    bool create(int64_t* pAttackerPlayerKey,
                int64_t* pDefenderPlayerKey,
                string* pMsg)
    {
        Value request = makeList(makeInt(1), makeInt(0));
        Value response;
        if (!makeRequest("/aliens/send", request, &response, pMsg))
        {
            return false;
        }

        if (!isList(response))
        {
            if (pMsg) *pMsg = "create: invalid response 1";
            return false;
        }
        vector<Value> list1 = getList(response);
        if (list1.size() < 1 ||
            !isInt(list1[0]))
        {
            if (pMsg) *pMsg = "create: invalid response 2";
            return false;
        }
        if (getInt(list1[0]) != 1)
        {
            if (pMsg) *pMsg = "create: protocol error";
            return false;
        }
        if (list1.size() < 2 ||
            !isList(list1[1]))
        {
            if (pMsg) *pMsg = "create: invalid response 3";
            return false;
        }
        vector<Value> list2 = getList(list1[1]);
        if (list2.size() != 2 ||
            !isList(list2[0]) ||
            !isList(list2[1]))
        {
            if (pMsg) *pMsg = "create: invalid response 4";
            return false;
        }
        vector<Value> list3 = getList(list2[0]);
        vector<Value> list4 = getList(list2[1]);
        if (list3.size() != 2 ||
            !isInt(list3[0]) ||
            !isInt(list3[1]) ||
            list4.size() != 2 ||
            !isInt(list4[0]) ||
            !isInt(list4[1]))
        {
            if (pMsg) *pMsg = "create: invalid response 5";
            return false;
        }
        int64_t iRole0 = getInt(list3[0]);
        int64_t iRole1 = getInt(list4[0]);
        if (iRole0 != 0 && iRole0 != 1)
        {
            if (pMsg) *pMsg = "create: invalid response 6";
            return false;
        }
        if (iRole1 != 0 && iRole1 != 1)
        {
            if (pMsg) *pMsg = "create: invalid response 7";
            return false;
        }
        if (iRole0 == 0 && iRole1 == 1)
        {
            if (pAttackerPlayerKey) *pAttackerPlayerKey = getInt(list3[1]);
            if (pDefenderPlayerKey) *pDefenderPlayerKey = getInt(list4[1]);
        }
        else if (iRole0 == 1 && iRole1 == 0)
        {
            if (pAttackerPlayerKey) *pAttackerPlayerKey = getInt(list4[1]);
            if (pDefenderPlayerKey) *pDefenderPlayerKey = getInt(list3[1]);
        }
        else
        {
            if (pMsg) *pMsg = "create: invalid response 8";
            return false;
        }

        return true;
    }

    bool join(int64_t playerKey,
              Info* pInfo,
              string* pMsg)
    {
        *pInfo = Info();

        Value request = makeList(makeInt(2),
                                 makeInt(playerKey),
                                 makeList(makeInt(192496425430)));
        Value response;
        if (!makeRequest("/aliens/send", request, &response, pMsg))
        {
            return false;
        }

        if (!isSuccess(response))
        {
            if (pMsg) *pMsg = "join: protocol error";
            return false;
        }

        if (!parseInfo(response, pInfo))
        {
            if (pMsg) *pMsg = "join: error parsing info";
            return false;
        }

        return true;
    }

    bool startTutorial(int64_t playerKey,
                       Info* pInfo,
                       State* pState,
                       string* pMsg)
    {
        *pInfo = Info();
        *pState = State();

        Value request = makeList(makeInt(3),
                                 makeInt(playerKey),
                                 makeNil());
        Value response;
        if (!makeRequest("/aliens/send", request, &response, pMsg))
        {
            return false;
        }

        if (!isSuccess(response))
        {
            if (pMsg) *pMsg = "startTutorial: protocol error";
            return false;
        }

        if (!parseInfo(response, pInfo))
        {
            if (pMsg) *pMsg = "startTutorial: error parsing info";
            return false;
        }

        if (!parseState(response, pState))
        {
            if (pMsg) *pMsg = "startTutorial: error parsing state";
            return false;
        }

        return true;
    }

    bool start(int64_t playerKey,
               const Params& params,
               Info* pInfo,
               State* pState,
               string* pMsg)
    {
        *pInfo = Info();
        *pState = State();

        Value request = makeList(makeInt(3),
                                 makeInt(playerKey),
                                 makeList(makeInt(params.m_fuel),
                                          makeInt(params.m_guns),
                                          makeInt(params.m_cooling),
                                          makeInt(params.m_ships)));
        Value response;
        if (!makeRequest("/aliens/send", request, &response, pMsg))
        {
            return false;
        }

        if (!isSuccess(response))
        {
            if (pMsg) *pMsg = "start: protocol error";
            return false;
        }

        if (!parseInfo(response, pInfo))
        {
            if (pMsg) *pMsg = "start: error parsing info";
            return false;
        }

        if (!parseState(response, pState))
        {
            if (pMsg) *pMsg = "start: error parsing state";
            return false;
        }

        return true;
    }

    bool play(int64_t playerKey,
              const vector<Command>& commands,
              Info* pInfo,
              State* pState,
              string* pMsg)
    {
        *pInfo = Info();
        *pState = State();

        Value request = makeList(makeInt(4),
                                 makeInt(playerKey),
                                 formatCommands(commands));
        Value response;
        if (!makeRequest("/aliens/send", request, &response, pMsg))
        {
            return false;
        }

        if (!isSuccess(response))
        {
            if (pMsg) *pMsg = "play: protocol error";
            return false;
        }

        if (!parseInfo(response, pInfo))
        {
            if (pMsg) *pMsg = "play: error parsing info";
            return false;
        }

        if (!parseState(response, pState))
        {
            if (pMsg) *pMsg = "play: error parsing state";
            return false;
        }

        return true;
    }

    bool getResult(int64_t playerKey,
                   string* pMsg)
    {
        Value request = makeList(makeInt(5),
                                 makeInt(playerKey));
        Value response;
        if (!makeRequest("/aliens/send", request, &response, pMsg))
        {
            return false;
        }

        if (!isSuccess(response))
        {
            if (pMsg) *pMsg = "getResult: protocol error";
            return false;
        }

        return true;
    }
}
