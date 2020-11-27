#include "sysutil.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <typeinfo>

namespace hirzel
{
	namespace sys
	{
		long long get_millis()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		long long get_seconds()
		{
			return std::chrono::duration_cast<std::chrono::seconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		void wait_for_key()
		{
			std::cout << "Press enter to continue... ";
			std::cin.get();
		}

		void thread_sleep(unsigned int milliseconds)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
		}
	} // namespace sysutil
} // namespace hirzel