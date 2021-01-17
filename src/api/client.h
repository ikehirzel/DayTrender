#pragma once

#include "../data/candle.h"

#include "../data/clienttypes.h"

#include <string>
#include <vector>

namespace hirzel
{
	class Plugin;
}

namespace daytrender
{
	class Client
	{
	private:
		bool bound = false;
		hirzel::Plugin* handle = nullptr;
		std::string filename, label;
		
		// init func

		typedef void (*init_func)(const std::vector<std::string>&);
		init_func init_ptr = nullptr;

		// api functions

		typedef void (*get_candles_func)(candleset&, const std::string&);
		get_candles_func get_candles_ptr = nullptr;

		typedef void (*get_account_info_func)(account_info&);
		get_account_info_func get_account_info_ptr = nullptr;

		typedef bool (*market_order_func)(const std::string&, double);
		market_order_func market_order_ptr = nullptr;

		// getters

		typedef const char* (*to_interval_func)(int);
		to_interval_func to_interval_ptr = nullptr;

		typedef int (*max_candles_func)();
		max_candles_func max_candles_ptr = nullptr;

		typedef double (*paper_fee_func)();
		paper_fee_func paper_fee_ptr = nullptr;

		typedef double (*paper_minimum_func)();
		paper_minimum_func paper_minimum_ptr = nullptr;

		typedef void (*backtest_intervals_func)(std::vector<int>&);
		backtest_intervals_func backtest_intervals_ptr = nullptr;

		typedef void (*get_error_func)(std::string&);
		get_error_func get_error_ptr;

	public:
		Client(const std::string& _label, const std::string& _filepath, const std::vector<std::string>& _credentials);
		~Client();
		// getters for client functions

		std::string to_interval(int interval) const;
		candleset get_candles(const std::string& ticker, int interval, int max = 0) const;
		account_info get_account_info() const;
		bool market_order(const std::string& ticker, double amount) const;

		// getters for constants

		int max_candles() const;
		double paper_fee() const;
		double paper_minimum() const;
		std::vector<int> backtest_intervals() const;
		std::string get_error() const;

		// inline getter functions

		inline bool all_bound() const { return bound; }
		inline const std::string& get_label() const { return label; }
		inline const std::string& get_filename() const { return filename; }
		inline hirzel::Plugin* gethandle() const { return handle; }
	};
}