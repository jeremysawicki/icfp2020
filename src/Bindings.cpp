#include "Bindings.hpp"
#include "Value.hpp"

void Bindings::resize(uint32_t size)
{
    uint32_t oldSize = m_values.size();
    m_values.resize(size);
    for (uint32_t i = oldSize; i < size; i++)
    {
        m_values[i].init();
    }
}
