#include "tradealgorithm.h"
#include <tinyplug.h>
#include <hirzel/fountain.h>

#define PROCESS_FUNCTION	"process"
#define GETNAME_FUNCTION	"getName"

namespace daytrender
{
	TradeAlgorithm::TradeAlgorithm(const std::string& filename)
	{
		this->filename = filename;
		this->handle = new tinyplug::Plugin(filename, { PROCESS_FUNCTION, GETNAME_FUNCTION });
		this->name = handle->execute<std::string>(GETNAME_FUNCTION);
		successf("Loaded algorithm: %s", name.c_str());
	}

	TradeAlgorithm::~TradeAlgorithm()
	{
		delete handle;
	}
	
	algorithm_data TradeAlgorithm::process(const candleset& candles, unsigned int index,
		unsigned int window)
	{
		algorithm_data out = handle->execute<algorithm_data, const candleset&, unsigned int>
			(PROCESS_FUNCTION, candles, index, window);
		return out;
	}
}
