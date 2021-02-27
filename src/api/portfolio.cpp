#include "portfolio.h"

namespace daytrender
{
	void Portfolio::update()
	{
		_client.update();

		for (Asset& asset : _assets)
		{
			asset.update(_client);
		}
	}

	void Portfolio::remove_asset(const std::string& ticker)
	{
		for (int i = 0; i < _assets.size(); i++)
		{
			if (_assets[i].ticker() == ticker)
			{
				_assets.erase(_assets.begin() + i);
				break;
			}
		}
	}

	double Portfolio::risk_sum() const
	{
		double sum = 0.0;
		for (const Asset& asset : _assets)
		{
			sum += asset.risk();
		}
		return sum;
	}
}