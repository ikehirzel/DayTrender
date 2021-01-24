#pragma once

#include "../data/candle.h"

#include "../data/accountinfo.h"

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
		bool _bound = false;
		bool _live = false;
		int _asset_count = 0;
		double _risk = 0.0;

		hirzel::Plugin* _handle = nullptr;
		
		std::string _filename;
		std::string _label;
		
		// init func

		bool (*_init)(const std::vector<std::string>&) = nullptr;

		// api functions

		// non returning
		bool (*_market_order)(const std::string&, double) = nullptr;
		bool (*_close_all_positions)() = nullptr;
		bool (*_set_leverage)(int) = nullptr;

		// returning
		bool (*_get_account_info)(AccountInfo&) = nullptr;
		bool (*_get_candles)(CandleSet&, const std::string&) = nullptr;
		bool (*_get_shares)(double&, const std::string&) = nullptr;
		bool (*_get_price)(double&, const std::string&) = nullptr;
		bool (*_market_open)(bool&) = nullptr;
		bool (*_to_interval)(const char*, int) = nullptr;

		// getters

		int (*_max_candles)() = nullptr;
		double (*_paper_fee)() = nullptr;
		double (*_paper_minimum)() = nullptr;
		void (*_backtest_intervals)(std::vector<int>&) = nullptr;
		void (*_get_error)(std::string&) = nullptr;

		void flag_error();
		bool func_ok(const char* label, void(*func)());

	public:
		Client(const std::string& _label, const std::string& _filepath,
			const std::vector<std::string>& _credentials, double _risk);
		~Client();

		// api functions

		// non returning
		bool market_order(const std::string& ticker, double amount);
		bool close_all_positions();
		bool set_leverage(int multiplier);

		CandleSet get_candles(const std::string& ticker, int interval, int max = 0);
		AccountInfo get_account_info();
		double get_shares(const std::string& ticker);
		double get_price(const std::string& ticker);
		bool market_open();
		std::string to_interval(int interval);

		// getters for constants

		inline double paper_fee() const { return _paper_fee(); }
		inline double paper_minimum() const { return _paper_minimum(); }
		inline int max_candles() const { return _max_candles(); }

		inline std::vector<int> backtest_intervals() const
		{
			std::vector<int> out;
			_backtest_intervals(out);
			return out;
		}

		inline std::string get_error() const
		{
			std::string error;
			_get_error(error);
			return error;
		}

		// inline getter functions

		inline bool bound() const { return _bound; }
		inline const std::string& label() const { return _label; }
		inline const std::string& filename() const { return _filename; }
		inline hirzel::Plugin* handle() const { return _handle; }
		inline double risk() const { return _risk; }
		inline void increment_assets() { _asset_count++; }
	};
}