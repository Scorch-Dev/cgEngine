#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>
#include <cstdint>

namespace sentinel
{

/**
 * NOTE: the clock is NOT the same
 * as the Timer class. The Timer measures
 * time in real time between update() calls
 * (or a running average of multiple update() calls).
 *
 * The Clock class is used for variable time scale
 * clocks that run on time lines on TOP of real-time.
 * This is useful for things like slowing an animation down,
 * speeding it up, reversing it, etc.
 */
class Clock
{
public:
	explicit Clock(float start_time_seconds = 0.0f, float time_scale=1.0f);

	//getters setters
	std::uint64_t getTimeTicks();	
	float getTimeScale();
	void setTimeScale(float time_scale);
	void pause();
	void resume();

	//update & deltas
	float timeDifference(const Clock& other);
	void update(float delta_time);
	void stepSingleFrame();

private:

	static inline std::uint64_t secondsToTicks(float time_seconds)
	{
		return(s_ticks_per_second * time_seconds);
	}
	//WARNING: only use for small values of time_ticks or float will wrap (<1 sec is safe)
	static inline float ticksToSeconds(std::uint64_t time_ticks)
	{
		return(time_ticks / s_ticks_per_second);
	}

	std::uint64_t m_time_ticks;
	float m_time_scale;
	bool m_is_paused;


	static float s_ticks_per_second;
	static const float TARGET_FPS;
};

}

#endif
