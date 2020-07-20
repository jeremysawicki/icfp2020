#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "Common.hpp"
#include "Game.hpp"

namespace Protocol
{
    void init();
    void cleanup();

    bool initAPIKey(bool verbose,
                    std::string* pMsg = nullptr);
    bool initDocker(const std::string& url,
                    bool verbose,
                    std::string* pMsg = nullptr);

    bool test(const std::string& playerKey,
              std::string* pResponse = nullptr,
              std::string* pMsg = nullptr);

    bool send(const std::string& request,
              std::string* pResponse = nullptr,
              std::string* pMsg = nullptr);

    bool createTutorial(int64_t tutorialNum, // 1-13
                        Role* pRole,
                        int64_t* pPlayerKey,
                        std::string* pMsg = nullptr);

    bool create(int64_t* pAttackerPlayerKey,
                int64_t* pDefenderPlayerKey,
                std::string* pMsg = nullptr);

    bool join(int64_t playerKey,
              Info* pInfo,
              std::string* pMsg = nullptr);

    bool startTutorial(int64_t playerKey,
                       Info* pInfo,
                       State* pState,
                       std::string* pMsg = nullptr);

    bool start(int64_t playerKey,
               const Params& params,
               Info* pInfo,
               State* pState,
               std::string* pMsg = nullptr);

    bool play(int64_t playerKey,
              const std::vector<Command>& commands,
              Info* pInfo,
              State* pState,
              std::string* pMsg = nullptr);

    bool getResult(int64_t playerKey,
                   std::string* pMsg = nullptr);
}

#endif
