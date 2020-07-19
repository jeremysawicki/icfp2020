#include "PassBot.hpp"

using std::vector;

PassBot::PassBot()
{
}

PassBot::~PassBot()
{
}

void PassBot::getParams(const Info& info,
                        Params* pParams)
{
    *pParams = {0, 0, 0, 1};
}
