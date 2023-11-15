#pragma once

#include <chrono>
#include <string>
#include <ctime>  
#include "Logging.h"

class ScopeTimer
{
private:
	std::chrono::steady_clock::time_point m_start_time;
	std::string m_timer_name;

public:
	ScopeTimer(std::string name)
	{
		m_timer_name = name;
		m_start_time = std::chrono::high_resolution_clock::now();
	}

	~ScopeTimer()
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - m_start_time;

		ReportInfo("%s finished computation in: %fs", m_timer_name.c_str(), elapsed_seconds.count());
	}
};

// # tells the compiler to put timerName into quotes to make string, while ## just combines 2 names to create 1
#define MeasureScopeTime(timerName) ScopeTimer timerName##_Timer(#timerName)