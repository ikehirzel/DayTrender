#include "algorithmdata.h"

#ifndef INDICATORS
#error INDICATORS must be defined
#endif

using namespace daytrender;

extern "C" int ranges_size() { return INDICATORS + 1; }
extern "C" void algorithm (AlgorithmData& out);

typedef void(*IndiFunc)(Indicator&, const CandleSet&, int);

const Indicator& _add_indicator(AlgorithmData& out, IndiFunc indi, const char* type, const char* label)
{
	if (out.size() >= out.capacity())
	{
		out.flag_error("attempted to add too many indicators");
		return out[0];
	}
	short i = out.size();
	out.increment_size();
	out[i].set_ident(type, label);

	indi(out[i], out.candles(), out.ranges()[i + 1]);
	return out[i];
}

void _init_algorithm(AlgorithmData& out, const char* label)
{
	out.set_label(label);
	const CandleSet& candles = out.candles();
	const std::vector<int>& ranges = out.ranges();
	if (candles.empty())
	{
		out.flag_error("no candles were passed to algorithm!");
	}
	else if (!ranges.size())
	{
		out.flag_error("algorithm data has not been initialized");
	}
	else if (ranges.size() != ranges_size())
	{
		out.flag_error("algorithm dataset size did not match expected size");
	}
}

#define add_indicator(out, func, label) _add_indicator(out, func, #func, label); if (out.error()) return
#define init_algorithm(out, label) _init_algorithm(out, label); if(out.error()) return