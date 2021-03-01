#pragma once

#include "asset.h"

#include <string>
#include <vector>
#include <picojson.h>

namespace daytrender
{
	class Portfolio
	{
	private:
		std::string _label;
		Client _client;
		std::vector<Asset> _assets;
		bool _live = false;
		bool _bound = false;

	public:
		Portfolio() = default;
		Portfolio(const std::string& label, const picojson::object& config, const std::string& dir);

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