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
void* operator new(size_t size) throw(std::bad_alloc);
void* operator new[](size_t size) throw(std::bad_alloc);
// Matching delete operators
void operator delete(void* ptr) throw();
void operator delete[](void* ptr) throw();

#endif // !MEMCHECKALLOCATOR_HPP
