#pragma once

#include <string>
#include <picojson.h>

namespace daytrender
{
	namespace server
	{
		bool init(const picojson::value& config, const std::string& dir);
		void start();
		void stop();
	}
}