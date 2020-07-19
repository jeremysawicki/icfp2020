#include "BotFactory.hpp"
#include "PassBot.hpp"

using std::string;
using std::vector;

namespace BotFactory
{
    vector<string> getList()
    {
        return
        {
            "pass"
        };
    }

    std::unique_ptr<Bot> create(const string& name)
    {
        if (name == "pass") return std::make_unique<PassBot>();

        return nullptr;
    }
}
