
#ifndef STRATEGY_DEFS_H
#define STRATEGY_DEFS_H

#include "../data/strategydata.h"
#include "api_versions.h"

#ifndef INDICATORS
#define INDICATORS
#error INDICATORS must be defined
#endif

#ifndef DATA_LENGTH
#define DATA_LENGTH
#error DATA_LENGTH must be defined
#endif

using namespace daytrender;

extern "C"
{
	int indicator_count() { return INDICATORS; }
	int data_length() { return DATA_LENGTH; }
	int api_version() { return STRATEGY_API_VERSION; }
	void strategy(StrategyData& out);
}
typedef void(*IndiFunc)(Indicator&, const CandleSet&, int);

const Indicator& _add_indicator(StrategyData& out, IndiFunc indi, const char* type, const char* label)
{
	if (out.size() >= out.capacity())
	{
		out.flag_error("attempted to add too many indicators");
		return out[0];
	}
	short i = out.size();
	out.increment_size();
	out[i].set_ident(type, label);

	indi(out[i], out.candles(), out.ranges()[i]);
	return out[i];
}

void _init_strategy(StrategyData& out, const char* label)
{
	out.set_label(label);
	const CandleSet& candles = out.candles();
	const std::vector<int>& ranges = out.ranges();
	if (candles.empty())
	{
		out.flag_error("no candles were passed to strategy!");
	}
	else if (!ranges.size())
	{
		out.flag_error("strategy data has not been initialized");
	}
	else if (ranges.size() != indicator_count())
	{
		out.flag_error("strategy dataset size did not match expected size");
	}
}

#define add_indicator(out, func, label) _add_indicator(out, func, #func, label); if (out.error()) return
#define init_strategy(out, label) _init_strategy(out, label); if(out.error()) return

#endif