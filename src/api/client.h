#ifndef DAYTRENDER_CLIENT_H
#define DAYTRENDER_CLIENT_H

// daytrender includes
#include "../data/candle.h"
#include "../data/accountinfo.h"
#include "../data/assetinfo.h"
#include "../data/result.h"

// standard library
#include <string>
#include <vector>
#include <unordered_map>

// external libararies
#include <hirzel/plugin.h>
#include <hirzel/data.h>

namespace daytrender
{
	class Client
	{
	private:
		static std::unordered_map<std::string, hirzel::Plugin*> _plugins;
		static hirzel::Plugin *get_plugin(const std::string& filename);

		hirzel::Plugin* _plugin = nullptr;
		std::string _filename;
		
		// init func

		bool (*_init)(const char **) = nullptr;

		// api functions

		// non returning
		
		bool (*_market_order)(const std::string&, double) = nullptr;
		bool (*_close_all_positions)() = nullptr;
		bool (*_set_leverage)(int) = nullptr;

		// returning
		bool (*_get_account_info)(AccountInfo&) = nullptr;
		bool (*_get_candles)(CandleSet&, const std::string&) = nullptr;
		bool (*_get_asset_info)(AssetInfo&, const std::string&) = nullptr;
		bool (*_secs_till_market_close)(int&) = nullptr;
		const char* (*_to_interval)(int) = nullptr;

		// getters

		int (*_key_count)() = nullptr;
		int (*_max_candles)() = nullptr;
		void (*_get_error)(std::string&) = nullptr;
		int (*_api_version)() = nullptr;

		void flag_error() const;
		bool client_ok() const;

		bool enter_position(const std::string& ticker, double pct, bool short_shares);
		bool exit_position(const std::string& ticker, bool short_shares);

	public:
		Client() = default;
		Client(const std::string& filename, const std::string& dir);

		static void free_plugins();

		// api functions

		bool init(const hirzel::Data& keys);

		// non returning
		bool market_order(const std::string& ticker, double amount);
		bool close_all_positions();
		bool set_leverage(unsigned leverage);

		// returning
		Result<AccountInfo> get_account_info() const;

		Result<CandleSet> get_candles(const std::string& ticker,
			unsigned interval, unsigned count) const;

		Result<AssetInfo> get_asset_info(const std::string& ticker) const;

		unsigned secs_till_market_close() const;
		std::string to_interval(int interval) const;

		// getters for constants

		int key_count() const { return _key_count(); }
		inline int max_candles() const { return _max_candles(); }
		inline unsigned api_version() const { return _api_version(); }

		inline std::string get_error() const
		{
			std::string error;
			_get_error(error);
			return error;
		}

		// inline getter functions
		inline bool is_bound() const { return !_plugin; }
		inline const std::string& filename() const { return _filename; }
	};
}

#endif
