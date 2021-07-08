#ifndef DAYTRENDER_CLIENT_H
#define DAYTRENDER_CLIENT_H

// daytrender includes
#include <data/asset.h>
#include <data/account.h>
#include <data/pricehistory.h>
#include <data/position.h>
#include <data/result.h>

// standard library
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// external libararies
#include <hirzel/plugin.h>
#include <hirzel/data.h>

#define cli_func_check() if (!_plugin) return "client is not bound"

namespace daytrender
{
	class Client
	{
	private:
		static std::unordered_map<std::string, std::shared_ptr<hirzel::Plugin>> _plugins;

		std::shared_ptr<hirzel::Plugin> _plugin;
		std::string _filename;
		
		// init func

		const char *(*_init)(const char **) = nullptr;

		// api functions
		
		const char *(*_market_order)(const char*, double) = nullptr;
		const char *(*_set_leverage)(uint32_t) = nullptr;
		const char *(*_get_account)(Account*) = nullptr;
		const char *(*_get_price_history)(PriceHistory*, const char*) = nullptr;
		const char *(*_get_position)(Position*, const char*) = nullptr;
		const char *(*_to_interval)(uint32_t) = nullptr;
		uint32_t(*_secs_till_market_close)() = nullptr;

		// getters

		uint32_t (*_key_count)() = nullptr;
		uint32_t (*_max_candles)() = nullptr;
		uint32_t (*_api_version)() = nullptr;


	public:
		Client() = default;
		Client(const std::string& filename, const std::string& dir);

		static void free_plugins();

		// api functions

		const char *init(const hirzel::Data& keys);

		// non returning
		const char *market_order(const std::string& ticker, double amount);
		const char *set_leverage(unsigned leverage);

		// returning
		Result<Account> get_account() const;

		Result<PriceHistory> get_price_history(const std::string& ticker,
			unsigned interval, unsigned count) const;

		Result<Position> get_position(const std::string& ticker) const;

		const char *to_interval(int interval) const;

		// derivative functions
		inline Result<PriceHistory> get_price_history(const Asset& asset) const
		{
			return get_price_history(asset.ticker(), asset.interval(), asset.candle_count());
		}

		const char *enter_position(const Asset& asset, double pct, bool short_shares);
		const char *exit_position(const Asset& asset, bool short_shares);
		const char *close_position(const Asset& asset);
		const char *close_all_positions(const std::vector<Asset>& assets);

		inline const char *enter_long(const Asset& asset, double pct)
			{ return enter_position(asset, pct, false); }
		inline const char *enter_short(const Asset& asset, double pct)
			{ return enter_position(asset, pct, true); }
		inline const char *exit_long(const Asset& asset)
			{ return exit_position(asset, false); }
		inline const char *exit_short(const Asset& asset)
			{ return exit_position(asset, true); }
		
		// getters for constants
		inline unsigned key_count() const { return _key_count(); }
		inline unsigned max_candles() const { return _max_candles(); }
		inline unsigned api_version() const { return _api_version(); }
		inline const char *to_interval(unsigned multiplier) const
		{
			if (!_plugin) return nullptr;
			return _to_interval((uint32_t)multiplier);
		}
		
		inline unsigned secs_till_market_close() const
		{
			if (!_plugin) return 0;
			return _secs_till_market_close();
		}

		// inline getter functions
		inline bool is_bound() const { return (bool)_plugin; }
		inline const std::string& filename() const { return _filename; }
	};
}

#endif
