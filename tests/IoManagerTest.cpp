#define CATCH_CONFIG_MAIN

#include <stdlib.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "catch.hpp"
#include "IoManager.h"

namespace s_test
{

using namespace sentinel;

TEST_CASE("Try IoManager read/writes to disk (muti & single-threaded)")
{
	static IoManager io;

	io.startUp();

	SECTION("Single threaded writing to \"test_single_thread.txt\" file")
	{
		FileHandle f_single_thread = io.openFile("test_single_thread.txt", true);
		REQUIRE(f_single_thread != NULL_FILE_HANDLE);

		//write some chars
		const char write_buf[] = "hello single threaded file!\n";
		FileOpStatus write_status;
		std::size_t bytes_written = io.writeFile(f_single_thread, write_buf,
			sizeof(write_buf), write_status);
		REQUIRE(write_status == success);

		//read them back in
		char read_buf[1024] = { 0 };
		FileOpStatus read_status;
		std::size_t bytes_read = io.readFile(f_single_thread, read_buf,
			sizeof(read_buf), read_status);
		REQUIRE(read_status == success);
        read_buf[bytes_read] = '\0';


		//compare to ensure no corruption
		REQUIRE(strcmp(read_buf, write_buf) == 0);

		io.closeFile(f_single_thread);
	}

	SECTION("multi threaded write to a single file \"test_multi_threaded.txt\"")
	{
		//create a few write strings and async write them
		std::vector<std::string> strs = { "These strings\n",
			"should be\n", "all separate\n",
			"and each\n", "should have\n", "its own line.\n" };

		//open up a file
		FileHandle f_multi_thread = io.openFile("test_multi_thread.txt",
			true);
		REQUIRE(f_multi_thread != NULL_FILE_HANDLE);

		//spin up some threads and make some requests
		std::vector<AsyncJobHandle> write_jobs;
		for (unsigned int i = 0; i < strs.size(); i++)
		{
			write_jobs.push_back(io.asyncWrite(f_multi_thread, strs[i].c_str(),
				strs[i].size(),
				[](FileOpStatus status, std::size_t bytes)
				{
					REQUIRE(status == IO_SUCCESS);
				}));
		}

		//wait for our threads to return
		for (unsigned int i = 0; i < write_jobs.size(); i++)
		{
			io.waitAsyncIo(write_jobs[i]);
		}

		//read them in and compare
		char read_buf[1024];
		AsyncJobHandle read_job = io.asyncRead(f_multi_thread,
			read_buf, sizeof(read_buf),
			[&read_buf](FileOpStatus status, std::size_t bytes)
            {
                REQUIRE(status == IO_SUCCESS);
                read_buf[bytes] = '\0';
            });


		io.waitAsyncIo(read_job);

        std::string read_str = std::string(read_buf);
        for(std::string s : strs)
            REQUIRE(read_str.find(s) != std::string::npos);

		io.closeFile(f_multi_thread);
	}

	SECTION("Try to write unbuffered and cancel at least one job.")
	{
		std::vector<std::string> strs = { "random", " strings",
		" and " , "yea, ", " you ", "get,", "the", " idea", "etc." ,
		"I ", "wonder", "if", "this ", "is," , "enough", "strings", 
		"to" , "maek", "this", "test" , "consistently ", "word", "???" };

		FileHandle f_multi_thread2 = io.openFile("test_multi_thread_w_cancel.txt",
			true);
		REQUIRE(f_multi_thread2 != NULL_FILE_HANDLE);

		//queue up a bunch of async writes unbuffered
		std::vector<AsyncJobHandle> write_jobs;
		for (unsigned int i = 0; i < strs.size(); i++)
		{
			write_jobs.push_back(io.asyncWriteUnbuffered(
                f_multi_thread2, strs[i].c_str(),
				strs[i].size(),
				[](FileOpStatus status, std::size_t bytes)
				{
					REQUIRE(status == IO_SUCCESS);
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}));
		}

		//try to sucesfully cancel at least one
		bool at_least_one_aborted = false;
		//NOTE: backwards b/c later jobs more likely to cancel
		for (unsigned int i = write_jobs.size(); i-- > 0; )
		{
			bool aborted = io.cancelAsyncIo(write_jobs[i]);

			if (aborted)
				at_least_one_aborted = true;
		}
		
		REQUIRE(at_least_one_aborted);

		for (unsigned int i = 0; i < write_jobs.size(); i++)
			io.waitAsyncIo(write_jobs[i]);

		io.closeFile(f_multi_thread2);

	}

	io.shutDown();
}

} //namespace s_test
