#pragma once

#include <string>
#include <picojson.h>

namespace daytrender
{
	namespace server
	{
		bool init(const picojson::object& config, const std::string& dir);
		void start();
		void stop();
	}
}