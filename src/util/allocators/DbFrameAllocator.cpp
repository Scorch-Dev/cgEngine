#include "DbFrameAllocator.h"


namespace sentinel
{

/*
 * ctor
 */
DbFrameAllocator::DbFrameAllocator(std::size_t stack_size_bytes) :
    m_buffers{ MemoryStack(stack_size_bytes) },
    m_current_buffer(0)
{
}

/*dtor*/
DbFrameAllocator::~DbFrameAllocator()
{
    //do nothing!
}

/*
 * clears the current buffer 
 */
void DbFrameAllocator::clearCurrentBuffer()
{
    m_buffers[m_current_buffer].clear();
}

/*
 * swaps the currently active buffer
 */
void DbFrameAllocator::swapBuffers()
{
    m_current_buffer = (m_current_buffer == 1) ? 0 : 1 ;
}

/*
 * gets a marker from the current buffer's top
 */
void* DbFrameAllocator::getStackPtr()
{
    return m_buffers[m_current_buffer].getStackPtr();
}

/*
 * allocates to curent buffer
 *
 * @param size_bytes: how many bytes to alloc
 */
void* DbFrameAllocator::alloc(std::size_t size_bytes)
{
    return m_buffers[m_current_buffer].alloc(size_bytes);
}

/*
 * allocates an alligned block of memory
 *
 * @param size_bytes: bytes to allocate
 * @param alignment: byte alignment
 */
void* DbFrameAllocator::allocAligned(std::size_t size_bytes, std::size_t alignment)
{
    return m_buffers[m_current_buffer].allocAligned(size_bytes, alignment);
}

/*
 * frees memory up to a marker
 */
void DbFrameAllocator::freeTo(void* marker)
{
    m_buffers[m_current_buffer].freeTo(marker);
}

/*
 * frees memory up to a marker gotten
 * after an aligned alloc
 */
void DbFrameAllocator::freeToAligned(void* marker)
{
    m_buffers[m_current_buffer].freeToAligned(marker);
}

} //namespace s_util
