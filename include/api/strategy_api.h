
#ifndef STRATEGY_API_H
#define STRATEGY_API_H

#include <data/chart.h>
#include <api/versions.h>
#include <api/action.h>

#ifndef LABEL
#define LABEL
#error LABEL must be defined
#endif

#ifndef DATA_LENGTH
#define DATA_LENGTH
#error DATA_LENGTH must be defined
#endif

using namespace daytrender;

#include <stdint.h>

struct IndicatorConfig
{
	void(*func)(Indicator&, const PriceHistory&, unsigned);
	const char *type;
	const char *label;
};

//extern std::vector<indicator_conf> indi_confs;
extern const std::vector<IndicatorConfig> config;

Action strategy(Chart& chart);
// api interface
extern "C"
{
	uint32_t indicator_count()
	{
		return config.size();
	}

	uint32_t data_length() { return DATA_LENGTH; }
	uint32_t api_version() { return STRATEGY_API_VERSION; }

	// user defined functions
	const char *execute(Chart* out)
	{
		Chart &chart = *out;
		chart.set_label(LABEL);

		if (chart.candles().empty())
			return "no candles were passed to strategy";

		if (chart.ranges().size() != indicator_count())
			return "strategy dataset size did not match expected sizse";

		for (size_t i = 0; i < config.size(); ++i)
		{
			chart[i].set_ident(config[i].type, config[i].label);
			config[i].func(chart[i], chart.candles(), chart.ranges()[i]);
		}

		Action act = strategy(chart);
		chart.set_action(act);

		return NULL;
	}
	// pre-defined functions
}

#endif
