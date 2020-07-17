#ifndef HEAP_HPP
#define HEAP_HPP

#include "Common.hpp"
#include "BitOps.hpp"

class Heap
{
public:
    Heap() :
        m_buckets(),
        m_mem(::malloc(1 * 1024 * 1024 * 1024)),
        m_nextAlloc((char*)m_mem + 16)
    {
    }

    ~Heap()
    {
        ::free(m_mem);
    }

    void* malloc(size_t size)
    {
        size_t bucket = bsr64(size | 8);
        void* p = m_buckets[bucket];
        if (p)
        {
            m_buckets[bucket] = *(void**)p;
        }
        else
        {
            p = m_nextAlloc;
            m_nextAlloc = (char*)m_nextAlloc + ((size_t)2 << bucket);
            *((uint8_t*)p - 1) = bucket;
        }
        return p;
    }

    void* calloc(size_t size)
    {
        void* p = malloc(size);
        memset(p, 0, size);
        return p;
    }

    void* calloc(size_t nmemb, size_t size)
    {
        size_t nmemb_size = nmemb * size;
        void* p = malloc(nmemb_size);
        memset(p, 0, nmemb_size);
        return p;
    }

    void free(void* p)
    {
        size_t bucket = *((uint8_t*)p - 1);
        *(void**)p = m_buckets[bucket];
        m_buckets[bucket] = p;
    }

    void* realloc(void* oldP, size_t size)
    {
        size_t bucket = *((uint8_t*)oldP - 1);
        size_t bucketSize = ((size_t)2 << bucket) - 1;
        if (size <= bucketSize)
        {
            return oldP;
        }
        else
        {
            void* newP = malloc(size);
            memcpy(newP, oldP, bucketSize);
            free(oldP);
            return newP;
        }
    }

private:
    void* m_buckets[64];
    void* m_mem;
    void* m_nextAlloc;
};

extern Heap heap;

#endif
