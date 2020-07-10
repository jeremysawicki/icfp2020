#ifndef CLEANUP_HPP
#define CLEANUP_HPP

#include "Common.hpp"

class Cleanup
{
public:
    explicit Cleanup(std::function<void()> func = nullptr) :
        m_func(std::move(func))
    {
    }

    Cleanup(Cleanup&& other) :
        m_func(std::move(other.m_func))
    {
        other.m_func = nullptr;
    }

    ~Cleanup()
    {
        if (m_func)
        {
            m_func();
        }
    }

    Cleanup& operator=(Cleanup&& other)
    {
        if (this != &other)
        {
            m_func = std::move(other.m_func);
            other.m_func = nullptr;
        }
        return *this;
    }

    void reset(std::function<void()> func = nullptr)
    {
        m_func = std::move(func);
    }

    void cleanup()
    {
        if (m_func)
        {
            m_func();
            m_func = nullptr;
        }
    }

private:
    std::function<void()> m_func;

private:
    Cleanup(const Cleanup& other) = delete;
    Cleanup& operator=(const Cleanup& other) = delete;
};

#endif
