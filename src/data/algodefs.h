#include "algotypes.h"

#ifndef INDICATORS
#error INDICATORS must be defined
#endif

using namespace daytrender;

extern "C" int ranges_size() { return INDICATORS + 1; }
extern "C" void algorithm (AlgorithmData& out);

typedef void(*IndiFunc)(Indicator&, const CandleSet&, int);

unsigned indicators = 0;

const Indicator& _add_indicator(AlgorithmData& out, IndiFunc indi, const char* type, const char* label)
{
	if (indicators >= out.size())
	{
		out.flag_error("attempted to add too many indicators");
		return out[0];
	}

	out[indicators].type = type;
	out[indicators].label = label;

	indi(out[indicators], out.candles, out.ranges[indicators + 1]);
	return out[indicators++];
}

void _init_algorithm(AlgorithmData& out, const char* label)
{
	out.set_label(label);
	indicators = 0;
	if (out.candles.empty())
	{
		out.flag_error("no candles were passed to algorithm!");
	}
	else if (!out.ranges.size())
	{
		out.flag_error("algorithm data has not been initialized");
	}
	else if (out.ranges.size() != ranges_size())
	{
		out.flag_error("algorithm dataset size did not match expected size");
	}
}

#define add_indicator(out, func, label) _add_indicator(out, func, #func, label); if (out.error()) return
#define init_algorithm(out, label) _init_algorithm(out, label); if(out.error()) return