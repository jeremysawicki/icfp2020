#ifndef XOSHIRO_HPP
#define XOSHIRO_HPP

#include "Common.hpp"
#include <random>

template<typename T, size_t N, typename Derived>
class XoshiroBase
{
public:
    typedef T result_type;

    static T min() { return 0; }
    static T max() { return -1; }

    void seed()
    {
        std::seed_seq seq;
        seed(seq);
    }

private:
    void seedHelper(uint32_t value)
    {
        std::seed_seq seq{value};
        seed(seq);
    }

    void seedHelper(uint64_t value)
    {
        std::seed_seq seq{(uint32_t)value, (uint32_t)(value >> 32)};
        seed(seq);
    }

public:
    void seed(T value)
    {
        seedHelper(value);
    }

    template<class Sseq>
    void seed(Sseq& seq)
    {
        size_t M = sizeof(T) / sizeof(uint32_t);
        uint32_t seeds[N * M];
        seq.generate(seeds, seeds + N * M);
        for (size_t i = 0; i < N; i++)
        {
            T si = 0;
            for (size_t j = 0; j < M; j++)
            {
                si |= (T)seeds[i * M + j] << (32 * j);
            }
            s[i] = si;
        }
    }

    void discard(unsigned long long n)
    {
        Derived* pThis = static_cast<Derived*>(this);
        while (n > 0)
        {
            pThis->operator()();
            n--;
        }
    }

    bool operator==(const XoshiroBase& other) const
    {
        for (size_t i = 0; i < N; i++)
        {
            if (s[i] != other.s[i])
            {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const XoshiroBase& other) const
    {
        return !(*this == other);
    }

protected:
    static T rotl(T x, int k)
    { return (x << k) | (x >> (sizeof(T) * 8 - k)); }

    T s[4];
};

class Xoshiro256PlusPlus : public XoshiroBase<uint64_t, 4, Xoshiro256PlusPlus>
{
public:
    Xoshiro256PlusPlus() { seed(); }
    explicit Xoshiro256PlusPlus(uint64_t value) { seed(value); }
    template<class Sseq>
    explicit Xoshiro256PlusPlus(Sseq& seq) { seed(seq); }

    uint64_t operator()()
    {
        uint64_t result = rotl(s[0] + s[3], 23) + s[0];

        uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rotl(s[3], 45);

        return result;
    }
};

class Xoshiro256StarStar : public XoshiroBase<uint64_t, 4, Xoshiro256StarStar>
{
public:
    Xoshiro256StarStar() { seed(); }
    explicit Xoshiro256StarStar(uint64_t value) { seed(value); }
    template<class Sseq>
    explicit Xoshiro256StarStar(Sseq& seq) { seed(seq); }

    uint64_t operator()()
    {
        uint64_t result = rotl(s[1] * 5, 7) * 9;

        uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rotl(s[3], 45);

        return result;
    }
};

class Xoshiro256Plus : public XoshiroBase<uint64_t, 4, Xoshiro256Plus>
{
public:
    Xoshiro256Plus() { seed(); }
    explicit Xoshiro256Plus(uint64_t value) { seed(value); }
    template<class Sseq>
    explicit Xoshiro256Plus(Sseq& seq) { seed(seq); }

    uint64_t operator()()
    {
        uint64_t result = s[0] + s[3];

        uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rotl(s[3], 45);

        return result;
    }
};

class Xoroshiro128PlusPlus : public XoshiroBase<uint64_t, 2, Xoroshiro128PlusPlus>
{
public:
    Xoroshiro128PlusPlus() { seed(); }
    explicit Xoroshiro128PlusPlus(uint64_t value) { seed(value); }
    template<class Sseq>
    explicit Xoroshiro128PlusPlus(Sseq& seq) { seed(seq); }

    uint64_t operator()()
    {
        uint64_t result = rotl(s[0] + s[1], 17) + s[0];

        s[1] ^= s[0];
        s[0] = rotl(s[0], 49) ^ s[1] ^ (s[1] << 21);
        s[1] = rotl(s[1], 28);

        return result;
    }
};

class Xoroshiro128StarStar : public XoshiroBase<uint64_t, 2, Xoroshiro128StarStar>
{
public:
    Xoroshiro128StarStar() { seed(); }
    explicit Xoroshiro128StarStar(uint64_t value) { seed(value); }
    template<class Sseq>
    explicit Xoroshiro128StarStar(Sseq& seq) { seed(seq); }

    uint64_t operator()()
    {
        uint64_t result = rotl(s[0] * 5, 7) * 9;

        s[1] ^= s[0];
        s[0] = rotl(s[0], 24) ^ s[1] ^ (s[1] << 16);
        s[1] = rotl(s[1], 37);

        return result;
    }
};

class Xoroshiro128Plus : public XoshiroBase<uint64_t, 2, Xoroshiro128Plus>
{
public:
    Xoroshiro128Plus() { seed(); }
    explicit Xoroshiro128Plus(uint64_t value) { seed(value); }
    template<class Sseq>
    explicit Xoroshiro128Plus(Sseq& seq) { seed(seq); }

    uint64_t operator()()
    {
        uint64_t result = s[0] + s[1];

        s[1] ^= s[0];
        s[0] = rotl(s[0], 24) ^ s[1] ^ (s[1] << 16);
        s[1] = rotl(s[1], 37);

        return result;
    }
};

class Xoshiro128PlusPlus : public XoshiroBase<uint32_t, 4, Xoshiro128PlusPlus>
{
public:
    Xoshiro128PlusPlus() { seed(); }
    explicit Xoshiro128PlusPlus(uint32_t value) { seed(value); }
    template<class Sseq>
    explicit Xoshiro128PlusPlus(Sseq& seq) { seed(seq); }

    uint32_t operator()()
    {
        uint32_t result = rotl(s[0] + s[3], 7) + s[0];

        uint32_t t = s[1] << 9;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rotl(s[3], 11);

        return result;
    }
};

class Xoshiro128StarStar : public XoshiroBase<uint32_t, 4, Xoshiro128StarStar>
{
public:
    Xoshiro128StarStar() { seed(); }
    explicit Xoshiro128StarStar(uint32_t value) { seed(value); }
    template<class Sseq>
    explicit Xoshiro128StarStar(Sseq& seq) { seed(seq); }

    uint32_t operator()()
    {
        uint32_t result = rotl(s[1] * 5, 7) * 9;

        uint32_t t = s[1] << 9;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rotl(s[3], 11);

        return result;
    }
};

class Xoshiro128Plus : public XoshiroBase<uint32_t, 4, Xoshiro128Plus>
{
public:
    Xoshiro128Plus() { seed(); }
    explicit Xoshiro128Plus(uint32_t value) { seed(value); }
    template<class Sseq>
    explicit Xoshiro128Plus(Sseq& seq) { seed(seq); }

    uint32_t operator()()
    {
        uint32_t result = s[0] + s[3];

        uint32_t t = s[1] << 9;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = rotl(s[3], 11);

        return result;
    }
};

#endif
