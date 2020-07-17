#ifndef BITOPS_HPP
#define BITOPS_HPP

#include "Common.hpp"

#define BITOPS_USE_ASM 1
#define BITOPS_ALLOW_TZCNT 1
#define BITOPS_ALLOW_LZCNT 1

// Bit Scan Forward
//
// Bit index of least significant 1 bit (undefined if input is 0)

inline uint64_t bsf64(uint64_t x)
{
#if BITOPS_USE_ASM && ARCH_64
    uint64_t y;
    __asm__ ( "tzcntq %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    return (uint64_t)__builtin_ctzll(x);
#endif
}

inline uint32_t bsf32(uint32_t x)
{
#if BITOPS_USE_ASM
    uint32_t y;
    __asm__ ( "tzcntl %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    return (uint32_t)__builtin_ctz(x);
#endif
}

// Bit Scan Reverse
//
// Bit index of most significant 1 bit (undefined if input is 0)

inline uint64_t bsr64(uint64_t x)
{
#if BITOPS_USE_ASM && ARCH_64
#if BITOPS_ALLOW_LZCNT
    uint64_t y;
    __asm__ ( "lzcntq %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y ^ 63;
#else
    uint64_t y;
    __asm__ ( "bsrq %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#endif
#else
    return (uint64_t)__builtin_clzll(x) ^ 63;
#endif
}

inline uint32_t bsr32(uint32_t x)
{
#if BITOPS_USE_ASM
#if BITOPS_ALLOW_LZCNT
    uint32_t y;
    __asm__ ( "lzcntl %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y ^ 31;
#else
    uint32_t y;
    __asm__ ( "bsrl %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#endif
#else
    return (uint32_t)__builtin_clz(x) ^ 31;
#endif
}

// Count Trailing Zero Bits
//
// Count the number of trailing least significant zero bits

inline uint64_t tzcnt64(uint64_t x)
{
#if BITOPS_USE_ASM && ARCH_64
#if BITOPS_ALLOW_TZCNT
    uint64_t y;
    __asm__ ( "tzcntq %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    // Relies on undefined behavior:
    // bsf leaves output unchanged when input is zero
    uint64_t y = 64;
    __asm__ ( "bsfq %1, %0" : "+r" (y) : "rm" (x) : "cc" );
    return y;
#endif
#else
    return x == 0 ? (uint64_t)64 : (uint64_t)__builtin_ctzll(x);
#endif
}

inline uint32_t tzcnt32(uint32_t x)
{
#if BITOPS_USE_ASM
#if BITOPS_ALLOW_TZCNT
    uint32_t y;
    __asm__ ( "tzcntl %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    // Relies on undefined behavior:
    // bsf leaves output unchanged when input is zero
    uint32_t y = 32;
    __asm__ ( "bsfl %1, %0" : "+r" (y) : "rm" (x) : "cc" );
    return y;
#endif
#else
    return x == 0 ? (uint32_t)32 : (uint32_t)__builtin_ctz(x);
#endif
}

// Count Leading Zero Bits
//
// Count the number of leading most significant zero bits

inline uint64_t lzcnt64(uint64_t x)
{
#if BITOPS_USE_ASM && ARCH_64
#if BITOPS_ALLOW_LZCNT
    uint64_t y;
    __asm__ ( "lzcntq %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    // Relies on undefined behavior:
    // bsr leaves output unchanged when input is zero
    uint64_t y = 127;
    __asm__ ( "bsrq %1, %0" : "+r" (y) : "rm" (x) : "cc" );
    return y ^ 63;
#endif
#else
    return x == 0 ? (uint64_t)64 : (uint64_t)__builtin_clzll(x);
#endif
}

inline uint32_t lzcnt32(uint32_t x)
{
#if BITOPS_USE_ASM
#if BITOPS_ALLOW_LZCNT
    uint32_t y;
    __asm__ ( "lzcntl %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    // Relies on undefined behavior:
    // bsr leaves output unchanged when input is zero
    uint32_t y = 63;
    __asm__ ( "bsrl %1, %0" : "+r" (y) : "rm" (x) : "cc" );
    return y ^ 31;
#endif
#else
    return x == 0 ? (uint32_t)32 : (uint32_t)__builtin_clz(x);
#endif
}

// Population Count
//
// Count the number of bits set to 1

inline uint64_t popcount64(uint64_t x)
{
#if BITOPS_USE_ASM && ARCH_64
    uint64_t y;
    __asm__ ( "popcntq %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    return __builtin_popcountll(x);
#endif
}

inline uint32_t popcount32(uint32_t x)
{
#if BITOPS_USE_ASM
    uint32_t y;
    __asm__ ( "popcntl %1, %0" : "=r" (y) : "rm" (x) : "cc" );
    return y;
#else
    return __builtin_popcount(x);
#endif
}

#endif
