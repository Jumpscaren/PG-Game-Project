#include "pch.h"
#include "Time.h"

double Time::GetDeltaTime(const Timer::TimeTypes& time_type)
{
	return s_delta_time / double(time_type);
}

double Time::GetElapsedTime()
{
	return s_elapsed_time / double(Timer::TimeTypes::Seconds);
}

void Time::Start()
{
	s_timer.StartTimer();
}

void Time::Stop()
{
	double time_change = s_timer.StopTimer();

	s_delta_time = time_change;
	s_elapsed_time += time_change;
}
