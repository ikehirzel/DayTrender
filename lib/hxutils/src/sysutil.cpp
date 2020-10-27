#include "sysutil.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <typeinfo>

namespace hirzel
{
	typedef std::chrono::milliseconds millis;
	typedef std::chrono::seconds seconds;
	
	long long getMillis()
	{
		return std::chrono::duration_cast<millis>(std::chrono::system_clock::now()
			.time_since_epoch()).count();
	}
	
	long long getSeconds()
	{
		return std::chrono::duration_cast<seconds>(std::chrono::system_clock::now()
			.time_since_epoch()).count();
	}
	
	void wait()
	{
		std::cout << "Press any key to continue..." << std::endl;
		std::cin.get();
	}
	
	void thread_sleep(unsigned int milliseconds)
	{
		std::this_thread::sleep_for(millis(milliseconds));
	}
}