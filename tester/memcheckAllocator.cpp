#include "memcheckAllocator.hpp"

void* operator new(size_t size) throw(std::bad_alloc) {
    if (TESTMEM) {
        if (++g_allocationCount == TESTMEM)
            throw std::bad_alloc();
    }
    void* ptr = malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void* operator new[](size_t size) throw(std::bad_alloc) {
    return operator new(size);
}

// Matching delete operators
void operator delete(void* ptr) throw() {
    free(ptr);
}

void operator delete[](void* ptr) throw() {
    operator delete(ptr);
}
