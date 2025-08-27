#ifndef MEMCHECKALLOCATOR_HPP
# define MEMCHECKALLOCATOR_HPP

#include <cstdlib>
#include <new>

// Global variables to control allocation behavior
// TESTMEM lets the nth memory allocation fail.
#ifndef TESTMEM
# define TESTMEM 0
#endif

static size_t g_allocationCount = 0;

// Overriding global new
void* operator new(size_t size) throw(std::bad_alloc) {
    if (TESTMEM) {
        if (++g_allocationCount == TESTMEM)
		{
            throw std::bad_alloc();
        }
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

#endif // !MEMCHECKALLOCATOR_HPP
