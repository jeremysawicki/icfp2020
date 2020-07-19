#ifndef TIMEUTILS_HPP
#define TIMEUTILS_HPP

#include "Common.hpp"

void sleepS(uint32_t durationS);
void sleepMS(uint32_t durationMS);

uint64_t getTimeS();
uint64_t getTimeMS();

#endif
