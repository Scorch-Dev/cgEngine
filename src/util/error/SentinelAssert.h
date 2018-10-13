#ifndef SENTINEL_ASSERT_H
#define SENTINEL_ASSERT_H

#include "SentinelConfig.h"

#ifdef ASSERTIONS_ENABLED

#include <iostream>
#include "MacroOverload.h"
#ifdef _MSC_VER
#include <Windows.h> //needed to __debugbreak()
#endif

namespace sentinel
{
void sentinelReportAssertFail(const char* expr_str, bool expr,
	const char* file, int line, const char* msg);
void debugBreak();
} //namespace sentinel

#define S_ASSERT2(expr, msg) \
    if(expr) {} \
    else \
    { \
        sentinel::sentinelReportAssertFail(#expr, expr, __FILE__, __LINE__, msg); \
		sentinel::debugBreak();\
    }


#define S_ASSERT1(expr) S_ASSERT2(expr, "")
#define S_ASSERT(...) CALL_OVERLOAD(S_ASSERT, __VA_ARGS__)

//optionally define Debug assert (DB_ASSERT) if in debug mode...
//use this for expensive asserts
#ifdef DEBUG
#define DB_ASSERT(...) S_ASSERT(...)

#else
#define DB_ASSERT(...) 

#endif //DEBUG

#else

//ASSERTS do nothing if no asserts are enabled
#define S_ASSERT(...)
#define DB_ASSERT(...) 

#endif //ASSERTIONS_ENABLED

#endif //SENTINEL_ASSERT_H
