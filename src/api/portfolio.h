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
		////////////>>
		bool _live = false;
		bool _shorting_enabled = false;
		int _asset_count = 0;
		double _risk_sum = 0.0;
		double _pl = 0.0;
		double _risk = 0.0;
		double _max_loss = 0.05;
		double _history_length = 24.0;
		int _closeout_buffer = 15 * 60;
		///////////<<

		std::string _label;
		Client _client;
		std::vector<Asset> _assets;
		bool _live = false;
		bool _bound = false;
		std::vector<std::pair<long long, double>> _equity_history;

	public:
		Portfolio() = default;
		Portfolio(const std::string& label, const hirzel::Data& config, const std::string& dir);

		void update();
		void remove_asset(const std::string& ticker);
		
		double risk_sum() const;

		inline void add_asset(const Asset& asset)
		{
			_assets.push_back(asset);
		}

		inline std::string label() const { return _label; }
		inline bool is_live() const { return _live; }
		inline bool is_bound() const { return _bound; }
	};
}

#endif