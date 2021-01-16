#pragma once

#include "../data/candle.h"

#include <string>
#include <vector>

namespace hirzel
{
	class Plugin;
}

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
	private:
		bool bound;
		hirzel::Plugin* handle = nullptr;
		std::string filename, label;
		std::vector<std::string> credentials;

		void (*to_interval_ptr)(std::string&, int);
		void (*get_candles_ptr)(candleset&, const std::string&, int, int);
		void (*get_account_info_ptr)(account_info&);
		double (*paper_fee_ptr)();
		double (*paper_minimum_ptr)();
		void (*backtest_intervals_ptr)(std::vector<int>&);

	public:
		TradeClient(const std::string& _label, const std::string& _filepath, const std::vector<std::string>& _credentials);

		// getters for client functions

		std::string to_interval(int interval) const;
		candleset get_candles(const std::string& ticker, int interval, int max = 0) const;
		account_info get_account_info() const;

		// getters for constants

		double paper_fee() const;
		double paper_minimum() const;
		std::vector<int> backtest_intervals() const;

		// inline getter functions

		inline bool all_bound() const { return bound; }
		inline const std::string& get_label() const { return label; }
		inline const std::string& get_filename() const { return filename; }
	};
}