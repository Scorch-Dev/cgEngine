#include "MemoryPool.h"

namespace sentinel
{

/**
 * ctor
 *
 * NOTE: no support for pools < 16bits item_size (and why would you?)
 *
 * @param item_size: size of each item
 * @param (optional) capacity: the starting capacity number of items
 *        (default is 128)
 */
MemoryPool::MemoryPool(std::size_t item_size, std::size_t capacity) :
    m_item_size(item_size),
    m_total_size((capacity + 1) * m_item_size), //first item is head of linked list
    m_pool(malloc(m_total_size))
{
    //alloc our poolm_n
	S_ASSERT(m_pool != nullptr);
	S_ASSERT((m_item_size > sizeof(uint16_t) || m_total_size < 16384), "Don't you DARE try to dynamically allocate one byte!");
	//NOTE: The 16384 is 2^16 - 1: aka max bytes representible using our algorithm for small items

    //linked list out of free memory blocks
    if(m_item_size >= sizeof(uintptr_t))
    {
        uintptr_t cast_pool = reinterpret_cast<uintptr_t>(m_pool);
        uintptr_t this_node;
        uintptr_t next_free_node;
        for(unsigned int i = 0; i < capacity; i++)
        {
            //modulo so tail points to head as sentinel
            next_free_node = cast_pool + 
                (((i+1) * m_item_size) % m_total_size);
            this_node = cast_pool + (i * m_item_size);

            *(reinterpret_cast<uintptr_t*>(this_node)) = next_free_node;
        }
    }
    //not enough for ptr? 16bit uint offsets as next address work too
    else
    {
        uintptr_t cast_pool = reinterpret_cast<uintptr_t>(m_pool);
        uint16_t* this_node;
        uint16_t next_offset;
        for(unsigned int i = 0; i < capacity; i++)
        {
            //modulo so final offset points to head as sentinel
            next_offset = (((i+1) * m_item_size) % m_total_size);
            this_node = reinterpret_cast<uint16_t*>(
                    cast_pool + (i * m_item_size));

            *(this_node) = next_offset;
        }
    }
}

MemoryPool::MemoryPool(MemoryPool&& other)
{
	LockT arg_lock(other.m_mut);

	m_item_size = other.m_item_size;
	m_total_size = other.m_total_size;
	m_pool = other.m_pool;

	other.m_item_size = std::size_t(0);
	other.m_total_size = std::size_t(0);
	other.m_pool = nullptr;
}

MemoryPool& MemoryPool::operator = (MemoryPool&& other)
{
	if (this != &other)
	{
		//need to lock both, then reassign
		LockT lhs_lock(m_mut, std::defer_lock);
		LockT rhs_lock(other.m_mut, std::defer_lock);
		std::lock(lhs_lock, rhs_lock);

		m_item_size = other.m_item_size;
		m_total_size = other.m_total_size;
		m_pool = other.m_pool;

		other.m_item_size = std::size_t(0);
		other.m_total_size = std::size_t(0);
		other.m_pool = nullptr;
	}
	return (*this);
}

/**
 * dtor
 */
MemoryPool::~MemoryPool()
{
	LockT lock(m_mut);

	if(m_pool) //in case was moved
		free(m_pool);
}

std::size_t MemoryPool::getItemSize() 
{
	return m_item_size;
}

/**
 * Returns a pointer corresponding to the first free slot.
 * If none is available, returns nullptr.
 *
 * @param n_bytes: the size in bytes
 * @returns void* to alloc'd item (never null)
 */
void* MemoryPool::alloc()
{
	LockT lock(m_mut);

    if(m_item_size >= sizeof(uintptr_t))
    {
        //pt head to the next of the next free
        uintptr_t* next_free_head = reinterpret_cast<uintptr_t*>(m_pool);
        uintptr_t* next_free = reinterpret_cast<uintptr_t*>( *next_free_head );

        if(next_free == next_free_head)
        {
            return nullptr; //points to head->full
        }
        uintptr_t second_next_free = *next_free;
        *next_free_head = second_next_free;

        return next_free;
    }
    else
    {
        //set offset to next free to next of next_free
        uint16_t* next_free_offset_head = reinterpret_cast<uint16_t*>(m_pool);
        uint16_t next_free_offset = (*next_free_offset_head);
        if( next_free_offset == 0 )
        {
            return nullptr; //point to head-> full
        }

        uintptr_t raw_next_free = reinterpret_cast<uintptr_t>(m_pool);
        raw_next_free += next_free_offset;
        uint16_t second_next_free_offset = *(
                reinterpret_cast<uint16_t*>(raw_next_free));

        *next_free_offset_head = second_next_free_offset;

        return (reinterpret_cast<void*>(raw_next_free));
    }
}

/**
 * Allocates a block of memory aligned ot a certain amount
 *
 * @param size_bytes: size of the actual memory block being allocated
 * @param alignment: the alignment in bytes of hte memory you're allocating
 */
void* MemoryPool::allocAligned(std::size_t size_bytes, std::size_t alignment)
{
    S_ASSERT(alignment >= 1);
    S_ASSERT(alignment <= 128);
    S_ASSERT((size_bytes + alignment) == m_item_size);

    //add alignment extra blocks & calculate alignment
    uintptr_t raw_address = (uintptr_t) alloc();
    if( reinterpret_cast<void*>(raw_address) == nullptr)
        return nullptr;

    std::size_t mask = (alignment - 1);
    uintptr_t misalignment = (raw_address & mask);
    std::ptrdiff_t adjustment = alignment - misalignment;

    S_ASSERT(adjustment < 256);
    uintptr_t aligned_address = raw_address + adjustment;
    
    //write the adjustment in the byte before the block
    uint8_t* adjustment_ptr = reinterpret_cast<uint8_t*>(aligned_address-1);
    *adjustment_ptr = adjustment;

    return (reinterpret_cast<void*>(aligned_address));
}

/**
 * free's an item from our pool allocator
 * 
 * @param p: a pointer to the memory to be freed
 */
void MemoryPool::freeBlock(void* p)
{
	//make sure that p actually allocated by this pool
    S_ASSERT(p != nullptr);
	S_ASSERT(reinterpret_cast<uintptr_t>(p) < (reinterpret_cast<uintptr_t>(m_pool) + m_total_size) );
	S_ASSERT(reinterpret_cast<uintptr_t>(p) > reinterpret_cast<uintptr_t>(m_pool)); //m_pool points to ll head, strict compare only

	LockT lock(m_mut);

    if(m_item_size >= sizeof(uintptr_t))
    {
        //p's data points to old next_free, and head's data points to p
        uintptr_t* next_free_head = reinterpret_cast<uintptr_t*>(m_pool);
        uintptr_t* new_node = reinterpret_cast<uintptr_t*>(p);

        uintptr_t next_free = *next_free_head;

        *new_node = next_free;
        *next_free_head = reinterpret_cast<uintptr_t>(new_node);
    }
    else
    {
        //p's data offsets to old next_free, and head's data points to p
        uint16_t* next_free_offset_head = reinterpret_cast<uint16_t*>(m_pool);
        uint16_t* new_node = reinterpret_cast<uint16_t*>(p);

        uint16_t next_free_offset = *next_free_offset_head;
        *new_node = next_free_offset;

        //need to manually count offset
        uint16_t hops = 0;
        uintptr_t raw_head = reinterpret_cast<uintptr_t>(m_pool);
        uintptr_t raw_p = reinterpret_cast<uintptr_t>(p);
        while(raw_p != raw_head)
		{
            hops++;
			raw_p = raw_p - 1;
		}

        *next_free_offset_head = hops;
    }
}

/**
 * grabs the 8bit adjustment stored before the aligned block
 * and readjusts the pointer to the beginning of the whole
 * block, then runs freeBlock(p).
 *
 * @param p: a pointer to the aligned memory block to free
 */
void MemoryPool::freeBlockAligned(void* p)
{
	//redundant double assert makes me cry :(
    S_ASSERT(p != nullptr);
	S_ASSERT(reinterpret_cast<uintptr_t>(p) < reinterpret_cast<uintptr_t>(m_pool) + m_total_size);
	S_ASSERT(reinterpret_cast<uintptr_t>(p) > reinterpret_cast<uintptr_t>(m_pool)); //m_pool points to ll head, strict compare only

    //adjust address back to raw form and free normally
    uintptr_t adjusted_address = reinterpret_cast<uintptr_t>(p);
    uint8_t adjustment = *(reinterpret_cast<uint8_t*>(adjusted_address - 1));
    uintptr_t raw_address = adjusted_address - adjustment;
    freeBlock(reinterpret_cast<void*>(raw_address));
}

}//namespace s_util
