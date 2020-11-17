#pragma once

#include <string>
#include <vector>

namespace daytrender
{
	namespace server
	{
		bool init(const std::string& dir);
		void start();
		void stop();
	}
}