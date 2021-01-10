#include "tradealgorithm.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

#define DAYTRENDER_ALGO_DIR "./res/algorithms/"

#define ALGO_FUNCTION	"algorithm"
#define COUNT_FUNCTION	"arg_count"

namespace daytrender
{
	TradeAlgorithm::TradeAlgorithm(const std::string& _filepath)
	{
		filename = hirzel::str::get_filename(_filepath);
		handle = new hirzel::Plugin(_filepath, { ALGO_FUNCTION, COUNT_FUNCTION });
		//args = handle->execute_return<int>(COUNT_FUNCTION);
		if (!handle->is_func_bound(ALGO_FUNCTION) || !handle->is_func_bound(COUNT_FUNCTION))
		{
			errorf(handle->get_error());
			return;
		}
		args = handle->execute<int>(COUNT_FUNCTION);
		algo = (algorithm_func)handle->get_func(ALGO_FUNCTION);
		bound = true;
		successf("Successfully loaded algorithm: %s", filename);
	}

	TradeAlgorithm::~TradeAlgorithm()
	{
		delete handle;
	}
	
	bool TradeAlgorithm::process(algorithm_data& data)
	{
		//printfmt("Executing algorithm...\n");
		if (!data.err.empty()) data.err.clear();
		data.dataset.clear();
		data.action = ACTION_NOTHING;

		if (!algo(data))
		{
			std::string args_glob;
			for (int i = 0; i < data.ranges.size(); i++)
			{
				if (i > 0) args_glob += ", ";
				args_glob += std::to_string(data.ranges[i]);
			}
			errorf("Error in Algorithm: %s(%s): %s", data.label, args_glob, data.err);

			return false;
		}
		else
		{
			return true;
		}
	}
}
