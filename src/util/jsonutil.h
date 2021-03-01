#pragma once

#include <vector>
#include <string>
#include <picojson.h>

using JsonValue = picojson::value;
using JsonObject = picojson::object;
using JsonArray = picojson::array;

namespace daytrender
{
	// value checks
	bool json_vars_are_defined(const JsonObject& config, const std::vector<std::string>& vars);
	bool json_vars_are_ratios(const JsonObject& config, const std::vector<std::string>& vars);
	bool json_vars_are_positive(const JsonObject& config, const std::vector<std::string>& vars);
	// type checks
	bool json_vars_are_arrays(const JsonObject& config, const std::string& var, int min);
	bool json_vars_are_strings(const JsonObject& config, const std::vector<std::string>& vars);
}