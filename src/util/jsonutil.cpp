#include "jsonutil.h"

#include <hirzel/fountain.h>
#include <nlohmann/json.hpp>

namespace daytrender
{	
	bool json_vars_are_defined(const nlohmann::json& config, const std::vector<std::string>& vars)
	{
		for (const std::string& var : vars)
		{
			if (config.find(var) == config.end())
			{
				ERROR("variable '%s' was not defined", var);
				return false;
			}
		}
		return true;
	}

	bool json_vars_are_ratios(const nlohmann::json& config, const std::vector<std::string>& vars)
	{
		for (const std::string& var : vars)
		{
			double val = config[var].get<double>();
			if (val > 1.0)
			{
				ERROR("variable '%s' was above 1.0", var);
				return false;
			}
			else if (val < 0.0)
			{
				ERROR("variable '%s' was below 0.0", var);
				return false;
			}
		}
		return true;
	}

	bool json_vars_are_positive(const nlohmann::json& config, const std::vector<std::string>& vars)
	{
		for (const std::string& var : vars)
		{
			double val = config[var].get<double>();
			if (val < 0.0)
			{
				ERROR("variable '%s' was non-positive", var);
				return false;
			}
		}
		return true;
	}
}