#pragma once

namespace hirzel
{
	namespace sys
	{
		// Get the current unix timestamp in milliseconds
		long long get_millis();
		// Get the current unix timestamp in seconds
		long long get_seconds();
		// Wait for input from stdin
		void wait_for_key();
		// Make the current thread sleep for specified milliseconds
		void thread_sleep(unsigned int milliseconds);
	}
}