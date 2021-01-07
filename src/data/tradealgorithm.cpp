#include "tradealgorithm.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

#define DAYTRENDER_ALGO_DIR "./res/algorithms/"

#define ALGO_FUNCTION	"algorithm"
#define COUNT_FUNCTION	"arg_count"

namespace daytrender
{
	TradeAlgorithm::TradeAlgorithm(const std::string& filepath)
	{
		filename = hirzel::str::get_filename(filepath);
		handle = new hirzel::Plugin(filepath, { ALGO_FUNCTION, COUNT_FUNCTION });
		args = handle->execute_return<int>(COUNT_FUNCTION);
		if (!args)
		{
			errorf("Failed to retrieve argument count from algorithm!");
			return;
		}
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

		if (!handle->execute_return<bool, algorithm_data&>(ALGO_FUNCTION, data))
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
