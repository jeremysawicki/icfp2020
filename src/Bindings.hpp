#ifndef BINDINGS_HPP
#define BINDINGS_HPP

#include "Common.hpp"

class Value;

class Bindings
{
public:
    void resize(uint32_t size);

    std::vector<Value> m_values;
    std::vector<uint32_t> m_order;
};

#endif
