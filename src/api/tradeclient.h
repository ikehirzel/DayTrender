#pragma once

#include "restclient.h"

#include <string>
#include <vector>

namespace daytrender
{
	struct candle;
	typedef std::vector<candle> candleset;

	class TradeClient
	{
	protected:
		std::vector<std::string> credentials;
	public:
		TradeClient(const std::vector<std::string>& credentials);
		virtual std::string toInterval(unsigned int interval) = 0;
		virtual candleset getCandles(const std::string& ticker,
			unsigned int interval, unsigned int max = 0) = 0;
	};
}