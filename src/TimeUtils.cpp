#include "TimeUtils.hpp"

#if PLATFORM_WINDOWS
#include <windows.h>
#else
#include <time.h>
#endif

void sleepS(uint32_t durationS)
{
    sleepMS(durationS * 1000);
}

void sleepMS(uint32_t durationMS)
{
#if PLATFORM_WINDOWS
    Sleep(durationMS);
#else
    struct timespec req;
    memset(&req, 0, sizeof(req));
    req.tv_sec = (time_t)(durationMS / 1000);
    req.tv_nsec = (long)((durationMS % 1000) * 1000000);
    nanosleep(&req, nullptr);
#endif
}

uint64_t getTimeS()
{
#if PLATFORM_WINDOWS
    return (uint64_t)GetTickCount64() / UINT64_C(1000);
#else
    struct timespec ts;
    memset(&ts, 0, sizeof(ts));
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0)
    {
        fprintf(stderr, "clock_gettime failed\n");
        exit(1);
    }
    uint64_t val = (uint64_t)ts.tv_sec;
    return val;
#endif
}

uint64_t getTimeMS()
{
#if PLATFORM_WINDOWS
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    memset(&ts, 0, sizeof(ts));
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0)
    {
        fprintf(stderr, "clock_gettime failed\n");
        exit(1);
    }
    uint64_t val = (uint64_t)ts.tv_sec;
    val *= 1000;
    val += ((uint64_t)ts.tv_nsec) / 1000000;
    return val;
#endif
}
