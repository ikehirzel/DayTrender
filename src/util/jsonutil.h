#pragma once

#include <vector>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

namespace daytrender
{
	bool json_vars_are_defined(const nlohmann::json& config, const std::vector<std::string>& vars);
	bool json_vars_are_ratios(const nlohmann::json& config, const std::vector<std::string>& vars);
	bool json_vars_are_positive(const nlohmann::json& config, const std::vector<std::string>& vars);
}