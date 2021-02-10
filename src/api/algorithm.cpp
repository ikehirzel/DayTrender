#include "algorithm.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

#define API_VERSION_CHECK
#include "algodefs.h"

namespace daytrender
{
	Algorithm::Algorithm(const std::string& _filepath)
	{
		_filename = hirzel::str::get_filename(_filepath);
		_plugin = new hirzel::Plugin(_filepath, { "indicator_count", "data_length", "algorithm", "api_version" });
		
		if (!_plugin->all_bound())
		{
			errorf(_plugin->get_error());
			return;
		}

		int api_version = _plugin->execute<int>("api_version");
		if (api_version != ALGORITHM_API_VERSION)
		{
			errorf("%s: api version (%d) did not match current api version: %d)", _filename, api_version, ALGORITHM_API_VERSION);
			return;
		}
		_indicator_count = _plugin->execute<int>("indicator_count");
		_data_length = _plugin->execute<int>("data_length");
		_algorithm_ptr = (void(*)(AlgorithmData&))_plugin->get_func("algorithm");
	}

	Algorithm::~Algorithm()
	{
		delete _plugin;
	}
	
	AlgorithmData Algorithm::process(const CandleSet& candles, const std::vector<int>& ranges) const
	{

		AlgorithmData data(ranges, candles);

		if (!_algorithm_ptr)
		{
			errorf("%s: algorithm function is not bound!", _filename);
			return data;
		}

		_algorithm_ptr(data);

		if (data.error())
		{
			std::string args_glob;
			const std::vector<int>& ranges = data.ranges();
			for (int i = 0; i < ranges.size(); i++)
			{
				if (i > 0) args_glob += ", ";
				args_glob += std::to_string(ranges[i]);
			}
			errorf("%s(%s): %s", data.label(), args_glob, data.error());
		}

		return data;
	}
}
