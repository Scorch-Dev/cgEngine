#include "MemoryStack.h"

namespace sentinel
{

MemoryStack::MemoryStack(std::size_t size_bytes) :
    m_total_size(size_bytes),
    m_stack_bottom( reinterpret_cast<uintptr_t>(malloc(size_bytes)) ),
    m_stack_top(m_stack_bottom + m_total_size),
    m_stack_marker(m_stack_bottom)
{
    if( !reinterpret_cast<void*>(m_stack_bottom) )
    {
        throw new std::bad_alloc();
    }
}

MemoryStack::MemoryStack(MemoryStack&& other)
{
	LockT arg_lock(other.m_mut);

	m_total_size = other.m_total_size;
	m_stack_bottom = other.m_stack_bottom;
	m_stack_top = other.m_stack_top;
	m_stack_marker = other.m_stack_marker;
	
	uintptr_t nullptr_raw = reinterpret_cast<uintptr_t>(nullptr);
	other.m_total_size = 0;
	other.m_stack_bottom = nullptr_raw;
	other.m_stack_top = nullptr_raw;
	other.m_stack_marker = nullptr_raw;
}

MemoryStack& MemoryStack::operator = (MemoryStack&& other)
{
	if (this != &other)
	{
		LockT lhs_lock(m_mut, std::defer_lock);
		LockT rhs_lock(other.m_mut, std::defer_lock);
		std::lock(lhs_lock, rhs_lock);

		m_total_size = other.m_total_size;
		m_stack_bottom = other.m_stack_bottom;
		m_stack_top = other.m_stack_top;
		m_stack_marker = other.m_stack_marker;

		uintptr_t nullptr_raw = reinterpret_cast<uintptr_t>(nullptr);
		other.m_total_size = 0;
		other.m_stack_bottom = nullptr_raw;
		other.m_stack_top = nullptr_raw;
		other.m_stack_marker = nullptr_raw;
	}
		
	return *(this);
}

MemoryStack::~MemoryStack()
{
	LockT lock(m_mut);
	if(reinterpret_cast<void*>(m_stack_bottom)) //in case it was moved
		free(reinterpret_cast<void*>(m_stack_bottom)); 
}


void* MemoryStack::getStackPtr()
{
	LockT lock(m_mut);
    return reinterpret_cast<void*>(m_stack_marker);
}

/**
 * Reservers some space, increment stack marer, 
 * and returns a pointer to the space. May return 
 * nullptr on stack full.
 * 
 * @param size_bytes: bytes to reserve
 * @return a pointer to the reserved data (or nullptr on full stack)
 */
void* MemoryStack::alloc(std::size_t size_bytes)
{
	LockT lock(m_mut);

    uintptr_t new_stack_marker = m_stack_marker + size_bytes;
    if(new_stack_marker > m_stack_top)
        return nullptr;

    void* mark = reinterpret_cast<void*>(m_stack_marker);
    m_stack_marker = new_stack_marker;
    return mark;
}

/**
 * Allocates an aligned block of memory.
 *
 * @param size_bytes: size of allocated block in bytes
 * @param alignment: alignment in bytes
 */
void* MemoryStack::allocAligned(std::size_t size_bytes, std::size_t alignment)
{
    S_ASSERT(alignment >= 1);
    S_ASSERT(alignment <=128);
    S_ASSERT((alignment & (alignment-1)) == 0); //pwr of 2

    std::size_t expanded_size_bytes = size_bytes + alignment; 

    //calculate adjusted address
    void* raw_ptr = alloc(expanded_size_bytes);
    uintptr_t raw_address = reinterpret_cast<uintptr_t>(raw_ptr);

    std::size_t mask = (alignment - 1);
    uintptr_t misalignment = (raw_address & mask);
    std::ptrdiff_t adjustment = alignment - misalignment;

    uintptr_t aligned_address = raw_address + adjustment;
    
    //write the adjustment to the preceding block
    S_ASSERT(adjustment < 256);
    uint8_t* adjustment_ptr = reinterpret_cast<uint8_t*>(aligned_address - 1);
    *adjustment_ptr = static_cast<uint8_t>(adjustment);

    return reinterpret_cast<void*>(aligned_address);
}

/**
 * Rolls back the stack marker to the specified marker
 *
 * @param marker: the marker to roll back to
 */
void MemoryStack::freeTo(void* marker)
{
    S_ASSERT(reinterpret_cast<uintptr_t>(marker) <= m_stack_marker);
    S_ASSERT(reinterpret_cast<uintptr_t>(marker) >= m_stack_bottom);

	LockT lock(m_mut);
    m_stack_marker = reinterpret_cast<uintptr_t>(marker);
}

/**
 * free to aligned memory ptr
 *
 * @param marker: a ptr to mem in this pool
 */
void MemoryStack::freeToAligned(void* marker)
{
    S_ASSERT(reinterpret_cast<uintptr_t>(marker) <= m_stack_marker);
    S_ASSERT(reinterpret_cast<uintptr_t>(marker) >= m_stack_bottom);

    uintptr_t raw_marker = reinterpret_cast<uintptr_t>(marker);
    uint8_t* adjustment_ptr = reinterpret_cast<uint8_t*>(raw_marker-1);
    uint8_t adjustment = *(adjustment_ptr);

    void* real_marker = reinterpret_cast<void*>( 
            reinterpret_cast<uintptr_t>(marker) - adjustment );
    freeTo(real_marker);
}

/**
 * Rolls back the stack marker to the beginning of the stack
 */
void MemoryStack::clear()
{
	LockT lock(m_mut);
    m_stack_marker = m_stack_bottom;
}

} //namespace sentinel
