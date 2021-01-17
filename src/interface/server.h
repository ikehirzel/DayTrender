#pragma once

#include <string>
#include <nlohmann/json.hpp>

using nlohmann::json;

namespace daytrender
{
	namespace server
	{
		bool init(const json& config, const std::string& dir);
		void start();
		void stop();
	}
}