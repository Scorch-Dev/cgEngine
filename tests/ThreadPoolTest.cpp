#define CATCH_CONFIG_MAIN

#include <vector>

#include "catch.hpp"
#include "ThreadPool.h"

namespace s_test
{

using namespace sentinel;

TEST_CASE("Test various thread pool basic functions")
{
	ThreadPool pool(4);

	SECTION("have threads do trivial math, and make sure they don't collide operations")
	{
		//add some numbers non-synchronously
		std::vector<int> nums_to_add = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		int answer = 0;
		for (int n : nums_to_add)
		{
			answer += n;
		}

		//now add them async
		int test_answer = 0;
		std::vector<AsyncJobHandle> jobs;
		for (int i = 0; i < nums_to_add.size(); i++)
		{
			AsyncJobHandle h = pool.asyncDo([&nums_to_add, &test_answer, i]() 
			{
				test_answer += nums_to_add[i];
			});
			jobs.push_back(h);
		}


		//check for double-add on job
		for (AsyncJobHandle h : jobs)
		{
			pool.wait(h);
		}
		REQUIRE(answer == test_answer);
	}
}

} //namespace s_test