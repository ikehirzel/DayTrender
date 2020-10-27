#pragma once

namespace hirzel
{
	// Get the current unix timestamp in milliseconds
	long long getMillis();
	// Get the current unix timestamp in seconds
	long long getSeconds();
	// Wait for input from stdin
	void wait();
	// Make the current thread sleep for specified milliseconds
	void thread_sleep(unsigned int milliseconds);
}