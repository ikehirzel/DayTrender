#ifndef PORTFOLIO_H
#define PORTFOLIO_H

// local includes
#include <data/asset.h>
#include <api/client.h>

//standard library
#include <string>
#include <vector>

// external libraries
#include <hirzel/data.h>

#define PORTFOLIO_UPDATE_INTERVAL 60

namespace daytrender
{
	class Portfolio
	{
	private: // data members

		bool _ok = false;
		double _pl = 0.0;
		long long _last_update = 0;

		bool _shorting_enabled = false;
		double _risk = 0.0;
		double _max_loss = 0.05;
		unsigned _history_length = 0;
		unsigned _closeout_buffer = 0;
		std::string _label;
		Client _client;
		std::vector<Asset> _assets;
		std::vector<std::pair<long long, double>> _equity_history;

	private: // initializer functions

		bool get_shorting_enabled(const hirzel::Data& config) const;
		double get_risk(const hirzel::Data& config) const;
		double get_max_loss(const hirzel::Data& config) const;
		double get_history_length(const hirzel::Data& config) const;
		unsigned get_closeout_buffer(const hirzel::Data& config) const;
		Client get_client(const hirzel::Data& config,
			const std::string& dir) const;
		std::vector<Asset> get_assets(const hirzel::Data& config,
			const std::string& dir) const;

	public: // public functions

		Portfolio(const hirzel::Data& config, const std::string& dir);

		void update();
		void update_assets();
		
		double risk_sum() const;

		Asset *get_asset(const std::string& ticker);

		/**
		 *	@return	State on whether its client and assets are bound 
		 */
		inline bool is_ok() const
		{
			return _ok;
		}

		inline bool should_update() const
		{
			// update once per minute
			return hirzel::sys::epoch_seconds() - _last_update >= PORTFOLIO_UPDATE_INTERVAL;
		}

		inline bool is_live() const
		{
			return (_client.secs_till_market_close() > _closeout_buffer);
		}

		inline Client& client()
		{
			return _client;
		}

		inline std::string label() const
		{
			return _label;
		}
	};
}

#endif