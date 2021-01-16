#include "algotypes.h"

#ifndef INDICATORS
#error INDICATORS must be defined
#endif

using namespace daytrender;

extern "C" int ranges_size() { return INDICATORS + 1; }
extern "C" void algorithm (algorithm_data& out);

typedef void(*indi_func)(indicator&, const candleset&, int);

unsigned indicators = 0;

const indicator& _add_indicator(algorithm_data& out, indi_func indi, const char* type, const char* label)
{
	if (indicators >= out.ranges_size - 1)
	{
		out.err = "attempted to add too many indicators";
		return out.dataset[0];
	}

	out.dataset[indicators].type = type;
	out.dataset[indicators].label = label;

	indi(out.dataset[indicators], out.candles, out.ranges[indicators + 1]);
	return out.dataset[indicators++];
}

void _init_algorithm(algorithm_data& out, const char* label)
{
	out.label = label;
	indicators = 0;
	if (out.candles.empty())
	{
		out.err = "no candles were passed to algorithm!";
	}
	else if (!out.ranges_size)
	{
		out.err = "algorithm data has not been initialized";
	}
	else if (out.ranges_size != ranges_size())
	{
		out.err = "algorithm dataset size did not match expected size";
	}
}

#define add_indicator(out, func, label) _add_indicator(out, func, #func, label); if (out.err) return
#define init_algorithm(out, label) _init_algorithm(out, label); if(out.err) return