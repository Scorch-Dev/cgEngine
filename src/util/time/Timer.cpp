#include "Timer.h"

namespace sentinel
{

/**
 * ctor
 * 
 * @param window_size: selects the window_size over
 *	which to perform the running average of delta time,
 *	which can be useful when timing something over many runs
 *	(e.g. the main game loop). Default value is 1, which means
 *	that it does no averaging and simply takes the difference.
 */
Timer::Timer(unsigned int window_size) :
	m_last_update_time(ClockT::now()),
	m_delta_time(0.0f),
	m_prev_deltas_window_size(window_size),
	m_prev_deltas(m_prev_deltas_window_size),
	m_frame_idx(0)
{
	S_ASSERT(window_size >= 1,
		"Must choose a positive non-zero window size for running average timer");
}

/**
 * @returns the time between this update
 *	and the last one in real-time seconds
 */
float Timer::deltaTime()
{
	return m_delta_time;
}

/**
 * updates the real-time clock based on last frame 

 *	Note, the ctor inits the prev-time field already,
 *	so you must call update() both before and after
 *	the block of code to time if you plan on NOT using
 *	the running average model. If you are using the running
 *	average (window_size > 1), then you should call update() once
 *	before the block, and then once after each iteration of
 * the block to time.
 */
void Timer::update()
{
	//grab length of this frame in seconds as float
	ClockT::time_point frame_end_time = ClockT::now();

	DurationT frame_duration = frame_end_time - m_last_update_time;

	m_last_update_time = frame_end_time;
	float this_frame_dt = frame_duration.count();

	//update running average (use windowed avg if we have enough samples)
	unsigned int current_end_idx = m_frame_idx % m_prev_deltas_window_size;

	if (m_frame_idx > m_prev_deltas_window_size)
		m_delta_time += ((this_frame_dt - m_prev_deltas[current_end_idx]) / m_prev_deltas_window_size);
	else
		m_delta_time += ( (this_frame_dt - m_delta_time) / (m_frame_idx + 1) );

	m_prev_deltas[current_end_idx] = this_frame_dt;
	m_frame_idx++;
}

}//namespace sentinel
