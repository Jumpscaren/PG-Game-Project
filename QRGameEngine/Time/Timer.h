#pragma once
class Timer
{
private:
	std::chrono::steady_clock::time_point m_startTime;
	std::chrono::steady_clock::time_point m_endTime;

public:
	enum class TimeTypes : uint64_t
	{
		Seconds = 1'000'000'000,
		Milliseconds = 1'000'000,
		Microseconds = 1'000,
		Nanoseconds = 1,
	};

public:
	Timer();

	void StartTimer();
	double StopTimer();
};

