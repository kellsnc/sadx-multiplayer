#pragma once

#include <chrono>

class Timer
{
public:
	explicit Timer(std::chrono::steady_clock::duration interval);
	bool Finished();
private:
	std::chrono::steady_clock::time_point m_time;
	std::chrono::steady_clock::duration m_interval;
};
