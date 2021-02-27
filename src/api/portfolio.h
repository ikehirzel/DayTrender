#pragma once

#include "asset.h"

#include <string>
#include <vector>

namespace daytrender
{
	class Portfolio
	{
	private:
		std::string _label;
		Client _client;
		std::vector<Asset> _assets;

	public:
		Portfolio() = default;

		void update();
		void remove_asset(const std::string& ticker);
		
		double risk_sum() const;

		inline void add_asset(const Asset& asset)
		{
			_assets.push_back(asset);
		}

		inline std::string label() const { return _label; }
	};
}