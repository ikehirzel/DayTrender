#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include "asset.h"

#include <string>
#include <vector>

#include <hirzel/data.h>

namespace daytrender
{
	class Portfolio
	{
	private:
		bool _live = false;
		bool _shorting_enabled = false;
		unsigned _asset_count = 0;
		double _risk_sum = 0.0;
		double _pl = 0.0;
		double _risk = 0.0;
		double _max_loss = 0.05;
		double _history_length = 24.0;
		unsigned _closeout_buffer = 15 * 60;
		///////////<<

		std::string _label;
		Client _client;
		std::vector<Asset> _assets;
		std::vector<std::pair<long long, double>> _equity_history;

		void enter_position(const std::string& ticker);
		void exit_position(const std::string& ticker);

	public:
		Portfolio() = default;
		Portfolio(const hirzel::Data& config, const std::string& dir);

		void update();
		void remove_asset(const std::string& ticker);
		
		double risk_sum() const;

		inline void add_asset(const Asset& asset)
		{
			_assets.push_back(asset);
		}

		inline std::string label() const { return _label; }

		/**
		 * @return	State of whether the portfolio is able to trade or not
		 */
		inline bool is_live() const { return _live; }

		/**
		 *	@return	State on whether its client and assets are bound 
		 */
		bool is_bound() const;
	};
}

#endif