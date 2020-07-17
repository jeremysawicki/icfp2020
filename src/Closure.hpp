#ifndef CLOSURE_HPP
#define CLOSURE_HPP

#include "Common.hpp"
#include "Function.hpp"
#include "Value.hpp"
#include "Heap.hpp"

class Closure
{
public:
    static Closure* create(size_t slotCount)
    {
        Closure* pClosure = (Closure*)heap.malloc(sizeof(Closure) - sizeof(Value) + sizeof(Value) * slotCount);
        pClosure->m_iInst = 0;
        pClosure->m_refCount = 1;
        pClosure->m_slotCount = slotCount;
        for (size_t i = 0; i < slotCount; i++)
        {
            new(&pClosure->m_freeEnv[i]) Value;
        }
        return pClosure;
    }

    void addRef()
    {
        m_refCount++;
    }

    void release()
    {
        if (--m_refCount == 0)
        {
            for (size_t i = 0; i < m_slotCount; i++)
            {
                m_freeEnv[i].~Value();
            }
            heap.free(this);
        }
    }

    uint32_t m_iInst;
    uint32_t m_refCount;
    uint32_t m_slotCount;
    Value m_freeEnv[1];
};

#endif
