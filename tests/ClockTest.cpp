#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <chrono>

#include "Clock.h"
#include "Timer.h"

namespace s_test
{
using namespace sentinel;

TEST_CASE("Verify that clocks update properly at various time scales")
{
	Timer real_time;

	SECTION("verify that slow clock tick slower than fast ones")
	{
		Clock slow_clock;
		slow_clock.setTimeScale(0.5f);

		Clock normal_clock;

		Clock fast_clock;
		fast_clock.setTimeScale(2.0f);

		real_time.update();
		for (int i = 0; i < 1000; i++)
		{
			float delta_time = real_time.deltaTime();

			slow_clock.update(delta_time);
			normal_clock.update(delta_time);
			fast_clock.update(delta_time);

			real_time.update();
		}

		//order by ticks, greatest to least: fast_clock, normal_clock, slow_clock
		REQUIRE(fast_clock.timeDifference(normal_clock) > 0);
		REQUIRE(normal_clock.timeDifference(slow_clock) > 0);

	}
}

}