#define CATCH_CONFIG_MAIN

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "catch.hpp"
#include "MemoryPool.h"
#include "DbFrameAllocator.h"
#include "SentinelAssert.h"


namespace s_test
{

using namespace sentinel;

/*
* allocates using a pool, storing pointers to allocated space in a vector.
* optionally, this can be done in an aligned fashion.
*
* @param arr: where to store allocated pointers
* @param pool: the pool to allocate to
* @param aligned: true if allocating aligned, else false
*/
void poolAllocSome(std::vector<void*>& arr, MemoryPool& pool, bool aligned)
{
	for (auto iter = arr.begin(); iter < arr.end(); iter++)
	{
		if (!aligned)
			*iter = pool.alloc();
		else if ( (pool.getItemSize() % 2) == 0)
			*iter = pool.allocAligned(pool.getItemSize() / 2, pool.getItemSize() / 2);
		else
			*iter = pool.allocAligned( (int)(pool.getItemSize() / 2) + 1, (int)((pool.getItemSize() / 2) + 1) );
	}
}

/*
* deallocates using a specified pool, freeing pointers specified by a vector.
* Can be used to free aligned or unaligned memory.
*
* @pram arr: vector<void*> pointers to dealloc
* @param pool: the pool to use
* @param aligned: true if aligned, false otherwise
*/
void poolDeallocSome(std::vector<void*>& arr, MemoryPool& pool, bool aligned)
{
	for (auto iter = arr.begin(); iter < arr.end(); iter++)
	{
		if (!aligned)
			pool.freeBlock(*iter);
		else
			pool.freeBlockAligned(*iter);
	}
}

/*
* validates allocations by a pool after memory has been alloced, dealloced, and
* then the same memory was alloced again. This ensures that, as per the algorithm
* the allocations the first time around are the reverse order of the allocations
* created during the second allocation.
*
* @param arr_1: vector<void*> allocated the first time
* @param arr_2: vector<void*> allocated the second time (after dealloc the first)
* @returns true if valid, otherwise false
*/
bool poolValidateAlloc(std::vector<void*>& arr_1, std::vector<void*>& arr_2)
{
	S_ASSERT(arr_1.size() == arr_2.size());

	for (std::size_t i = 0; i < arr_1.size(); i++)
	{
		if (arr_1[i] != arr_2[arr_1.size() - i -1])
		{
			return false;
		}
	}
	return true;
}

/*
* allocates 100 blocks to a stack & returns
* last alloced block (no reason to check all the pointers,
* b/c it's a stack, only the last one)
*
* @param s: the MemoryStack to use
* @param aligned: false if unaligned, true otherwise
* @returns a void* to the last item aligned
*/
void* stackAlloc100(DbFrameAllocator& frame, bool aligned)
{
	void* t;
	for (int i = 0; i < 500; i++)
	{
		if (!aligned)
			t = frame.alloc(sizeof(char*));
		else
			t = frame.allocAligned(sizeof(char*), sizeof(char*));
	}
	return t;
}

TEST_CASE("various allocation tests on various pools of different item sizes")
{
	std::vector<void*> pool_allocs_first(5);
	std::vector<void*> pool_allocs_second(5);
	MemoryPool pool(sizeof(uintptr_t) * 2);
	MemoryPool pool2(sizeof(uint16_t));
	
	//pool items are large
	SECTION("pool of large items: item_size >= sizeof(void*)")
	{
		SECTION("unaligned allocations")
		{
			//multiple test allocs for large(er) item algo
			poolAllocSome(pool_allocs_first, pool, false);
			poolDeallocSome(pool_allocs_first, pool, false);

			poolAllocSome(pool_allocs_second, pool, false);
			poolDeallocSome(pool_allocs_second, pool, false);

			REQUIRE(poolValidateAlloc(pool_allocs_first, pool_allocs_second));
		}
		SECTION("aligned pool allocations")
		{
			//de/alloc using large-item algo aligned
			poolAllocSome(pool_allocs_first, pool, true);
			poolDeallocSome(pool_allocs_first, pool, true);

			poolAllocSome(pool_allocs_second, pool, true);
			poolDeallocSome(pool_allocs_second, pool, true);

			REQUIRE(poolValidateAlloc(pool_allocs_first, pool_allocs_second));
		}
	}

	//small pool items (but not atomic)
	SECTION("pool of small items: 16bits <= item_size < sizeof(void*)")
	{
		SECTION("unaligned pool allocations")
		{
			//de/alloc using small-item algo non-aligned
			poolAllocSome(pool_allocs_first, pool2, false);
			poolDeallocSome(pool_allocs_first, pool2, false);
			poolAllocSome(pool_allocs_second, pool2, false);
			poolDeallocSome(pool_allocs_second, pool2, false);

			REQUIRE(poolValidateAlloc(pool_allocs_first, pool_allocs_second));
		}
		SECTION("aligned pool allocations")
		{
			//de/alloc using small-item algo aligned
			poolAllocSome(pool_allocs_first, pool2, true);
			poolDeallocSome(pool_allocs_first, pool2, true);

			poolAllocSome(pool_allocs_second, pool2, true);
			poolDeallocSome(pool_allocs_second, pool2, true);

			REQUIRE(poolValidateAlloc(pool_allocs_first, pool_allocs_second));
		}
	}
}

TEST_CASE("various allocation tests on on a stack allocator")
{
    DbFrameAllocator frame;
	
	SECTION("testing simple bulk allocation then clear()")
	{
		SECTION("not losing data on consecutive unaligned stack allocations")
		{
			//test allocation of non-aligned memory
			void* t1 = stackAlloc100(frame, false);
			S_ASSERT(t1 != nullptr);
			frame.clearCurrentBuffer();

			void* t2 = stackAlloc100(frame, false);
			S_ASSERT(t2 != nullptr);

			REQUIRE(t2 == t1); //not losing data
			frame.clearCurrentBuffer();
		}

		SECTION("not losing data on consecutive aligned stack allocations")
		{
			//test aligned allocation
			void* t3 = stackAlloc100(frame, true);
			S_ASSERT(t3 != nullptr);
			frame.clearCurrentBuffer();
			void* t4 = stackAlloc100(frame, true);
			S_ASSERT(t4 != nullptr)

			REQUIRE(t3 == t4); //make sure not losing data
			frame.clearCurrentBuffer();
		}
	}
	SECTION("allocations followed by partial clears")
	{
		SECTION("stack unaligned allocation, partial clear, then reallocation")
		{
			//test half clear roll backs for non-aligned
			void* t5 = stackAlloc100(frame, false);
			stackAlloc100(frame, false);
			frame.freeTo(t5);
			void* t6 = frame.getStackPtr();
			REQUIRE(t5 == t6);
			frame.clearCurrentBuffer();
		}

		SECTION("stack unaligned allocation, partial clear, then reallocation")
		{
			//test half clear roll backs for aligned
			void* t7 = stackAlloc100(frame, true);
			stackAlloc100(frame, true);
			frame.freeTo(t7);

			void* t8 = frame.getStackPtr();
			REQUIRE(t7 == t8);
			frame.clearCurrentBuffer();
		}
	}
}

}
