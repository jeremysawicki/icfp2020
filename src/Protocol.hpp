#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "Common.hpp"

namespace Protocol
{
    void init();
    void cleanup();

    bool test(const std::string& url,
              const std::string& playerKey,
              std::string* pResponse = nullptr,
              std::string* pMsg = nullptr);

    bool send(const std::string& request,
              std::string* pResponse = nullptr,
              std::string* pMsg = nullptr);
}

#endif
