#ifndef MEMORY_STACK_H
#define MEMORY_STACK_H

#include <cstddef>
#include <cstdint>
#include <new>
#include <mutex>
#include <thread>
#include <utility>

#include "SentinelAssert.h"

namespace sentinel
{

class MemoryStack
{
public:
    //TODO: this will need to be tweaked default throughout development
    MemoryStack(std::size_t size_bytes=625000); //5Mb test value default
	MemoryStack(const MemoryStack& other)              = delete;
	MemoryStack& operator = (const MemoryStack& other) = delete;
	MemoryStack(MemoryStack&& other);
	MemoryStack& operator = (MemoryStack&& other);
    ~MemoryStack();

    void* getStackPtr();
    void* alloc(std::size_t size_bytes);
    void* allocAligned(std::size_t size_bytes, std::size_t alignment);

    void freeTo(void* marker);
    void freeToAligned(void* marker);
    void clear();

private:
    std::size_t m_total_size;
    uintptr_t m_stack_bottom;
    uintptr_t m_stack_top;
    uintptr_t m_stack_marker;

	typedef std::mutex MutexT;
	typedef std::unique_lock<MutexT> LockT;

	MutexT m_mut;
};

} //namespace s_util

#endif
