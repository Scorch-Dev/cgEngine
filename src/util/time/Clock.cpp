#include "Clock.h"

namespace sentinel
{

float Clock::s_ticks_per_second = std::chrono::high_resolution_clock::period::den / std::chrono::high_resolution_clock::period::num;;
const float Clock::TARGET_FPS = 60.0f;

/**
 * ctor
 * defaults clock to running
 * NOTE: be sure to call the static init() function before
 *		 creating your first Clock object.
 *
 * @param start_time_seconds: start time local to this clock
 *        (default: t=0)
 * @param time_scale: the time_scale of the clock
 *        (default: 1.0)
 */
Clock::Clock(float start_time_seconds, float time_scale) :
	m_time_ticks(secondsToTicks(start_time_seconds)),
	m_time_scale(time_scale),
	m_is_paused(false)
{}

/**
 * returns time since the clock was
 * instantiated, scaled by its time_scale.
 */
std::uint64_t Clock::getTimeTicks()
{
	return m_time_ticks;
}

/*time scale of the clock (relative to real time)*/
float Clock::getTimeScale()
{
	return m_time_scale;
}

void Clock::setTimeScale(float time_scale)
{
	m_time_scale = time_scale;
}

/*pauses and unpauses the clock. easy enough...*/
void Clock::pause()
{
	m_is_paused = true;
}

void Clock::resume()
{
	m_is_paused = false;
}

/*calculates difference between number of
 * ticks recorded by this clock at its time scale,
 * and ticks recorded by another clock at its time_scale.\
 * NOTE: this will fail for large time differences
 * (on the order of  than one second)
 *
 * @returns the difference between the two in seconds.
 */
float Clock::timeDifference(const Clock& other)
{
	std::uint64_t delta_time_ticks = m_time_ticks - other.m_time_ticks;
	return ticksToSeconds(delta_time_ticks);
}

/**
 * updates this clock's internal state
 * (if not paused)
 *
 * @param delta_time: the length of time
 *		  occupied by the last frame
 */
void Clock::update(float delta_time)
{
	if (!m_is_paused)
	{
		std::uint64_t scaled_delta_time_ticks =
			secondsToTicks(delta_time * m_time_scale);
		m_time_ticks += scaled_delta_time_ticks;
	}
}

/**
 * advances clock by the expected duration of one frame
 * (e.g. 1  / the target fps time the scale factor).
 */
void Clock::stepSingleFrame()
{
	if (!m_is_paused)
	{
		std::uint64_t scaled_delta_time_ticks =
			secondsToTicks( (1.0f/TARGET_FPS) * m_time_scale );
		m_time_ticks += scaled_delta_time_ticks;
	}
}

}//namespace sentinel