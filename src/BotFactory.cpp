#include "BotFactory.hpp"
#include "PassBot.hpp"
#include "OrbitBot.hpp"
#include "ShootBot.hpp"
#include "CloneBot.hpp"

using std::string;
using std::vector;

namespace BotFactory
{
    vector<string> getList()
    {
        return
        {
            "pass",
            "orbit",
            "shoot",
            "clone"
        };
    }

    std::unique_ptr<Bot> create(const string& name)
    {
        if (name == "pass") return std::make_unique<PassBot>();
        if (name == "orbit") return std::make_unique<OrbitBot>();
        if (name == "shoot") return std::make_unique<ShootBot>();
        if (name == "clone") return std::make_unique<CloneBot>();

        return nullptr;
    }
}
