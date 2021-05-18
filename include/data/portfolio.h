#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include "asset.h"

#include <string>
#include <vector>

#include <hirzel/data.h>

#define PORTFOLIO_UPDATE_INTERVAL 60

namespace daytrender
{
	class Portfolio
	{
	private:
		// status flag
		bool _ok = false;
		// update timer
		long long _last_update = 0;

		// portfolio settings
		bool _shorting_enabled = false;
		unsigned _asset_count = 0;
		double _risk_sum = 0.0;
		double _pl = 0.0;
		double _risk = 0.0;
		double _max_loss = 0.05;
		double _history_length = 24.0;
		unsigned _closeout_buffer = 15 * 60;



		std::string _label;
		Client _client;
		std::vector<Asset> _assets;
		std::vector<std::pair<long long, double>> _equity_history;

		void enter_position(const Asset& asset, bool short_shares);
		void exit_position(const Asset& asset, bool short_shares);

	public:
		Portfolio() = default;
		Portfolio(const hirzel::Data& config, const std::string& label,
			const std::string& dir);

		void update();
		void update_assets();
		inline bool should_update() const
		{
			// update once per minute
			return hirzel::sys::epoch_seconds() - _last_update >= PORTFOLIO_UPDATE_INTERVAL;
		}
		
		double risk_sum() const;

		Asset *get_asset(const std::string& ticker);
		inline Client& get_client() { return _client; }
		inline std::string label() const { return _label; }

		/**
		 *	@return	State on whether its client and assets are bound 
		 */
		inline bool is_ok() const { return _ok; }
		inline bool is_live() const
		{
			return (_client.secs_till_market_close() > _closeout_buffer);
		}
	};
}

#endif