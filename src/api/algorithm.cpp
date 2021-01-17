#include "algorithm.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

#define DAYTRENDER_ALGO_DIR "./res/algorithms/"

#define ALGO_FUNCTION	"algorithm"
#define COUNT_FUNCTION	"ranges_size"

namespace daytrender
{
	Algorithm::Algorithm(const std::string& _filepath)
	{
		filename = hirzel::str::get_filename(_filepath);
		handle = new hirzel::Plugin(_filepath, { ALGO_FUNCTION, COUNT_FUNCTION });
		
		if (!handle->all_bound())
		{
			errorf(handle->get_error());
			return;
		}

		ranges_count = handle->execute<int>(COUNT_FUNCTION);
		algo = (algorithm_func)handle->get_func(ALGO_FUNCTION);
		bound = true;
		successf("Successfully loaded algorithm: '%s'", filename);
	}

	Algorithm::~Algorithm()
	{
		delete handle;
	}
	
	algorithm_data Algorithm::process(const candleset& candles, const std::vector<int>& ranges) const
	{
		//printfmt("Executing algorithm...\n");
		algorithm_data data;
		data.create(ranges.data(), ranges_count);
		data.candles = candles;

		algo(data);

		if (data.err)
		{
			std::string args_glob;
			for (int i = 0; i < data.ranges_size; i++)
			{
				if (i > 0) args_glob += ", ";
				args_glob += std::to_string(data.ranges[i]);
			}
			errorf("%s(%s): %s", data.label, args_glob, data.err);
		}

		return data;
	}
}
