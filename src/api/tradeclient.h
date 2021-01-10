#pragma once

#include "restclient.h"

#include <string>
#include <vector>

namespace daytrender
{
	struct account_info
	{
		double balance = 0.0;
		double buying_power = 0.0;
		double equity = 0.0;
	};

	class TradeClient
	{
	protected:
		std::vector<std::string> credentials;
	public:
		TradeClient(const std::vector<std::string>& credentials);
		virtual std::string toInterval(unsigned int interval) = 0;
		virtual candleset getCandles(const std::string& ticker,
			unsigned int interval, unsigned int max = 0) = 0;
		virtual account_info getAccountInfo() const = 0;
	};
}