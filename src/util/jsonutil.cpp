#include "jsonutil.h"

#include <hirzel/fountain.h>

using JsonValue = picojson::value;
using JsonObject = picojson::object;

namespace daytrender
{	
	bool json_vars_are_defined(const JsonObject& config, const std::vector<std::string>& vars)
	{
		bool all_defined = true;
		for (const std::string& var : vars)
		{
			if (config.find(var) == config.end())
			{
				ERROR("value '%s' is not defined", var);
				all_defined = false;
			}
		}
		return all_defined;
	}

	bool json_vars_are_strings(const JsonObject& config, const std::vector<std::string>& vars)
	{
		bool all_strings = true;
		for (const std::string& var : vars)
		{
			if (!config.at(var).is<std::string>())
			{
				ERROR("value '%s' is not a string", var);
				all_strings = false;
			}
		}
		return all_strings;
	}

	bool json_vars_are_ratios(const JsonObject& config, const std::vector<std::string>& vars)
	{

		// for (const std::string& var : vars)
		// {
		// 	// checking if it is defined
		// 	if (config.find(var) == config.end())
		// 	{
		// 		ERROR("value '%s' was not defined", var);
		// 		return false;
		// 	}

		// 	const Json& var_json = config[var];
		// 	auto type = var_json.type();

		// 	if (type != json::value_t::number_float && type != json::value_t::number_unsigned && type != json::value_t::number_integer)
		// 	{
		// 		ERROR("value '%s' is not a number", var);
		// 		return false;
		// 	}

		// 	double val = var_json.get<double>();
		// 	if (val > 1.0 )
		// 	{
		// 		ERROR("value '%s' is above 1.0", var);
		// 		return false;
		// 	}
		// 	else if (val < 0.0)
		// 	{
		// 		ERROR("value '%s' is below 0.0", var);
		// 		return false;
		// 	}
		// }
		return true;
	}

	bool json_vars_are_positive(const JsonObject& config, const std::vector<std::string>& vars)
	{
		// for (const std::string& var : vars)
		// {
		// 	if (config.find(var) == config.end())
		// 	{
		// 		ERROR("value '%s' is not defined", var);
		// 		return false;
		// 	}

		// 	const Json& var_json = config[var];
		// 	auto type = var_json.type();
		// 	if (type != json::value_t::number_float && type != json::value_t::number_unsigned && type != json::value_t::number_integer)
		// 	{
		// 		ERROR("value '%s' is not a number", var);
		// 	}

		// 	double val = config[var].get<double>();
		// 	if (val < 0.0)
		// 	{
		// 		ERROR("variable '%s' was negative", var);
		// 		return false;
		// 	}
		// }
		return true;
	}
}