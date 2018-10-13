#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "IoManager.h"
#include "Logger.h"

namespace s_test
{

using namespace sentinel;

TEST_CASE("try logging to a test logger")
{
	static Logger logger;
	static IoManager io;

	//try to start up
	io.startUp();
	logger.startUp("test_log.txt", io);

	//blocking log
	for (int i = 0; i < 10; i++)
	{
		logger.log("This is a blocking log!");
		logger.logAsync("This is a non-blocking log!");
		logger.logAsync("This is a blocking log, written immediately!\n", true);
	}

	//shutdown
	logger.shutDown();
	io.shutDown();
}
}//namespace s_test