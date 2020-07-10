#include "Protocol.hpp"
#include "Cleanup.hpp"
#include "StringUtils.hpp"
#include <curl/curl.h>

// sudo apt-get install libcurl4-openssl-dev

using std::string;
using std::vector;
using std::pair;

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

    size_t writeFunction(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        string* pStr = (string*)userdata;
        pStr->append(ptr, size * nmemb);
        return size * nmemb;
    }

    bool makeRequest(const string& url,
                     const vector<pair<string, string>>* pQueryData,
                     bool usePost,
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

        string queryStr;
        if (pQueryData)
        {
            bool first = true;
            for (auto& queryItem : *pQueryData)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    queryStr += '&';
                }

                {
                    char* key = curl_easy_escape(curl, queryItem.first.c_str(), queryItem.first.size());
                    queryStr += key;
                    curl_free(key);
                }

                queryStr += '=';

                {
                    char* value = curl_easy_escape(curl, queryItem.second.c_str(), queryItem.second.size());
                    queryStr += value;
                    curl_free(value);
                }
            }
        }

        string fullUrl = url;
        if (usePost)
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, queryStr.c_str());
        }
        else
        {
            fullUrl += '?';
            fullUrl += queryStr;
        }

        char errorBuf[CURL_ERROR_SIZE];

        curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuf);

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

    bool test(const string& url,
              const string& playerKey,
              string* pResponse,
              string* pMsg)
    {
        vector<pair<string, string>> queryData;
        queryData.emplace_back("playerKey", playerKey);
        if (!makeRequest(url,
                         &queryData,
                         false,
                         pResponse,
                         pMsg))
        {
            return false;
        }

        return true;
    }
}
