#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstdint>
#include <cstdlib>
#include <list>
#include <mutex>
#include <new>
#include <thread>
#include <vector>

#include "SentinelAssert.h"

namespace sentinel
{

class MemoryPool
{
public:
    MemoryPool(std::size_t item_size, std::size_t capacity=128);
	MemoryPool(const MemoryPool& other)              = delete;
	MemoryPool& operator = (const MemoryPool& other) = delete;
	MemoryPool(MemoryPool&& other);
	MemoryPool& operator = (MemoryPool&& other);
    ~MemoryPool();

	std::size_t getItemSize();

    void* alloc();
    void* allocAligned(std::size_t size_bytes, std::size_t alignment);
    void freeBlock(void* p);
    void freeBlockAligned(void* p);

private:
    std::size_t m_item_size;
    std::size_t m_total_size;
    void* m_pool;

	typedef std::unique_lock<std::mutex> LockT;
	typedef std::mutex MutexT;

	MutexT m_mut;
};

} //namespace s_util

#endif
