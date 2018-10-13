#ifndef DB_FRAME_ALLOCATOR_H
#define DB_FRAME_ALLOCATOR_H

#include "MemoryStack.h"

namespace sentinel
{

class DbFrameAllocator
{
public:
    DbFrameAllocator(std::size_t stack_size_bytes=625000);
    DbFrameAllocator(const DbFrameAllocator& other)              = delete;
    DbFrameAllocator& operator = (const DbFrameAllocator& other) = delete;
    DbFrameAllocator(DbFrameAllocator&& other)                   = default;
    DbFrameAllocator& operator = (DbFrameAllocator&& other)      = default;
    ~DbFrameAllocator();
    
    void clearCurrentBuffer();
    void swapBuffers();

    void* getStackPtr();
    void* alloc(std::size_t size_bytes);
    void* allocAligned(std::size_t size_bytes, std::size_t alignment);

    void freeTo(void* marker);
    void freeToAligned(void* marker);

private:
    MemoryStack m_buffers[2];
    std::size_t m_current_buffer;
};

} //namespace s_util

#endif
