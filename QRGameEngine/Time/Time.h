#pragma once
#include "Timer.h"

class Time
{
private:
	static inline Timer s_timer;
	static inline double s_delta_time;
	static inline double s_elapsed_time;

public:
	static double GetDeltaTime(const Timer::TimeTypes& time_type = Timer::TimeTypes::Seconds);
	static double GetElapsedTime();

	static void Start();
	static void Stop();
};

