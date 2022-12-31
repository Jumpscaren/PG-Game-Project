#include "pch.h"
#include "Timer.h"

Timer::Timer()
{
	StartTimer();
}

void Timer::StartTimer()
{
	m_startTime = std::chrono::steady_clock::now();
}

double Timer::StopTimer()
{
	m_endTime = std::chrono::steady_clock::now();

	return std::chrono::duration_cast<std::chrono::nanoseconds>(m_endTime - m_startTime).count();
}
