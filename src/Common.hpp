#ifndef COMMON_HPP
#define COMMON_HPP

#if defined(_WIN32)
#define PLATFORM_WINDOWS 1
#if defined(_WIN64)
#define ARCH_64 1
#else
#define ARCH_64 0
#endif
#else
#define PLATFORM_WINDOWS 0
#if defined(__x86_64__)
#define ARCH_64 1
#elif defined(__i386__)
#define ARCH_64 0
#else
#error "Can't determine if architecture is 32-bit or 64-bit"
#endif
#endif

#include <stddef.h>
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if PLATFORM_WINDOWS
#define PRIuZ "Iu"
#define PRIoZ "Io"
#define PRIxZ "Ix"
#define PRIXZ "IX"
#define PRIdT "Id"
#define PRIiT "Ii"
#define PRIoT "Io"
#define PRIxT "Ix"
#define PRIXT "IX"
#else
#define PRIuZ "zu"
#define PRIoZ "zo"
#define PRIxZ "zx"
#define PRIXZ "zX"
#define PRIdT "td"
#define PRIiT "ti"
#define PRIoT "to"
#define PRIxT "tx"
#define PRIXT "tX"
#endif

#include <string>
#include <vector>
#include <set>
#include <map>
#include <functional>
#include <memory>
#include <utility>
#include <algorithm>

#endif
