#include "SentinelAssert.h"

namespace sentinel
{

#ifdef ASSERTIONS_ENABLED

/**
* logs the error to cerr (only that for now)
*
* @param expr_str: the condition as a string
* @param expr: the expression to evaluate (a boolean)
* @param file: the file where the error was detected
* @param line: the line where the error was detected
* @param msg: a custom message to display
*/
void sentinelReportAssertFail(const char* expr_str, bool expr,
	const char* file, int line, const char* msg)
{
	std::cerr << "Assert failed:\t" << msg << "\n"
		<< "Expected:\t" << expr_str << "\n"
		<< "Source:\t\t" << file << ", line " << line << "\n";
}

/*
* break into the debugger depending on platform
*/
void debugBreak()
{
#ifdef _MSC_VER
	__debugbreak();
#elif __GNUC__  || __clang__
	asm("int $3");
#endif
}

#endif //ASSERTIONS_ENABLED

} //namespace sentinel
