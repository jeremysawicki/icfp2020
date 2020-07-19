#ifndef BOTFACTORY_HPP
#define BOTFACTORY_HPP

#include "Common.hpp"

class Bot;

namespace BotFactory
{
    std::vector<std::string> getList();
    std::unique_ptr<Bot> create(const std::string& name);
}

#endif
