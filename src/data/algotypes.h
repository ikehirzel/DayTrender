#pragma once

#define ACTION_NOTHING	0U
#define ACTION_SELL		1U
#define ACTION_BUY		2U

#include <string>
#include <vector>
#include <unordered_map>
#include "candle.h"

namespace daytrender
{
	struct indicator_data
	{
		std::string type, label;
		std::vector<double> data;
	};
	
	struct algorithm_data
	{
		std::string type, label, err;
		std::vector<int> ranges;
		candleset candles;
		std::vector<indicator_data> dataset;
		int action;
		
		void do_nothing() { action = ACTION_NOTHING; }
		void sell() { action = ACTION_SELL; }
		void buy() { action = ACTION_BUY; }
	};
}
