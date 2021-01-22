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
		int asset_count = 0;
		double risk = 0.0;
		
		// init func

		void (*init_ptr)(const std::vector<std::string>&) = nullptr;

		// api functions

		void (*get_candles_ptr)(CandleSet&, const std::string&) = nullptr;
		void (*get_account_info_ptr)(AccountInfo&) = nullptr;
		bool (*market_order_ptr)(const std::string&, double) = nullptr;
		double (*get_shares_ptr)(const std::string&) = nullptr;
		bool (*close_all_positions_ptr)() = nullptr;
		bool (*market_open_ptr)() = nullptr;
		double (*get_price_ptr)(const std::string&) = nullptr;
		double (*get_leverage_ptr)() = nullptr;
		bool (*set_leverage_ptr)(int) = nullptr;

		// getters

		const char* (*to_interval_ptr)(int) = nullptr;
		int (*max_candles_ptr)() = nullptr;
		double (*paper_fee_ptr)() = nullptr;
		double (*paper_minimum_ptr)() = nullptr;
		void (*backtest_intervals_ptr)(std::vector<int>&) = nullptr;
		void (*get_error_ptr)(std::string&) = nullptr;

		void flag_error();

	public:
		Client(const std::string& _label, const std::string& _filepath,
			const std::vector<std::string>& _credentials, double _risk);
		~Client();

		// api functions

		CandleSet get_candles(const std::string& ticker, int interval, int max = 0) const;
		AccountInfo get_account_info() const;
		bool market_order(const std::string& ticker, double amount) const;
		double get_shares(const std::string& ticker) const;
		bool close_all_positions() const;
		bool market_open() const;
		double get_price(const std::string& ticker) const;
		double get_leverage() const;
		bool set_leverage(int numerator) const;

		// getters for constants

		std::string to_interval(int multiplier) const;
		double paper_fee() const;
		double paper_minimum() const;
		std::vector<int> backtest_intervals() const;
		int max_candles() const;
		std::string get_error() const;

		// inline getter functions

		inline bool all_bound() const { return bound; }
		inline const std::string& get_label() const { return label; }
		inline const std::string& get_filename() const { return filename; }
		inline hirzel::Plugin* gethandle() const { return handle; }
		inline double get_risk() const { return risk; }
		inline int get_asset_count() const { return asset_count; }
		inline void set_asset_count(int count) { asset_count = count; }
		inline double get_asset_share() const { return risk / (double)asset_count; }
	};
}