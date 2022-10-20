#include "pch.h"
#include <chrono>
#include "timer.h"

Timer::Timer(std::chrono::steady_clock::duration interval)
{
	m_time = std::chrono::steady_clock::now();
	m_interval = interval;
}

bool Timer::Finished()
{
	auto now = std::chrono::steady_clock::now();
	auto elapsed = now - m_time;

	if (elapsed >= m_interval)
	{
		m_time = now;
		return true;
	}
	return false;
}