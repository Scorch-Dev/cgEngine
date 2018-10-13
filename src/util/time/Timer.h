#ifndef REAL_TIME_H
#define REAL_TIME_H

#include <chrono>
#include <vector>

#include "SentinelAssert.h"

namespace sentinel
{

/*
 * note the timer is not a utility
 * for use in subsystems, but rather times
 * a block of code in real-time.
 * WARNING: This only works well for small
 * blocks of time, and will likely drift or wrap
 * for durations > 1-2 seconds
 */
class Timer
{
public:
	Timer(unsigned int window_size=1);

	float deltaTime();
	void update();

private:
	typedef std::chrono::high_resolution_clock ClockT;
	typedef std::chrono::duration<float> DurationT;

	ClockT::time_point m_last_update_time;
	float m_delta_time;

	//circular vector implementation for running average
	unsigned int m_prev_deltas_window_size;
	std::vector<float> m_prev_deltas;
	unsigned long m_frame_idx;
};

}//namespace sentinel

#endif //REAL_TIME_H