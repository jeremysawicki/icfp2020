#include "Heap.hpp"
#include "Int.hpp"

Heap heap;

#if INT_IMPL_GMP

#include <gmpxx.h>

void* heapAllocate(size_t size)
{
    return heap.malloc(size);
}

void* heapReallocate(void* p, size_t oldSize, size_t newSize)
{
    return heap.realloc(p, newSize);
}

void heapFree(void* p, size_t size)
{
    heap.free(p);
}

class InitGmpHeap
{
public:
    InitGmpHeap()
    {
        mp_set_memory_functions(heapAllocate, heapReallocate, heapFree);
    }
} initGmpHeap;

#endif
