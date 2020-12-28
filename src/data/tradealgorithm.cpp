#include "tradealgorithm.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>

#define PROCESS_FUNCTION	"process"
#define GETNAME_FUNCTION	"getName"

#define DAYTRENDER_ALGO_DIR "./res/algorithms/"

namespace daytrender
{
	TradeAlgorithm::TradeAlgorithm(const std::string& filename)
	{
		this->filename = filename;
		handle = new hirzel::Plugin(filename, { PROCESS_FUNCTION, GETNAME_FUNCTION });
		name = handle->execute_return<std::string>(GETNAME_FUNCTION);
		if(name.empty())
		{
			errorf("Failed to load algorithm: %s", filename);
			return;
		}
		bound = true;
		successf("Successfully loaded algorithm: %s", name);
	}

	TradeAlgorithm::~TradeAlgorithm()
	{
		delete handle;
	}
	
	algorithm_data TradeAlgorithm::process(const candleset& candles)
	{
		algorithm_data data;
		handle->execute_void<algorithm_data&, const candleset&>(PROCESS_FUNCTION, data, candles);
		return data;
	}
}
