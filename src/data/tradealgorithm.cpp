#include "tradealgorithm.h"
#include <tinyplug.h>

#define PROCESS_FUNCTION	"process"
#define GETNAME_FUNCTION	"getName"

#define TRADEALGO_NAME		"Trade Algo"

namespace daytrender
{
	TradeAlgorithm::TradeAlgorithm(const std::string& filename)
	{
		l = hirzel::Logger(TRADEALGO_NAME);
		this->filename = filename;
		this->handle = new tinyplug::Plugin(filename, { PROCESS_FUNCTION, GETNAME_FUNCTION });
		this->name = handle->execute<std::string>(GETNAME_FUNCTION);
		l.success("Loaded algorithm: " + name);
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
