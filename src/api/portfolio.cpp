#include "portfolio.h"

#include "../util/jsonutil.h"

namespace daytrender
{
	Portfolio::Portfolio(const std::string& label, const JsonObject& config, const std::string& dir) :
	_label(label)
	{
		if (!json_vars_are_defined(config,
		{
			"client",
			"assets"
		})) return;

		if (!json_vars_are_ratios(config, {
			"max_loss",
			"risk"
		})) return;

		if (!json_vars_are_positive(config, {
			"history_length",
			"leverage",
			"closeout_buffer"
		})) return;

		_client = Client(config.at("client").get<JsonObject>(), dir);

		const JsonArray& assets_json = config.at("assets").get<JsonArray>();
		_assets.resize(assets_json.size());
		for (int i = 0; i < assets_json.size(); i++)
		{
			Asset a(assets_json[i].get<JsonObject>(), dir);
			if (!a.is_live()) return;
			_assets[i] = a;
		}

		_bound = true;
		_live = true;
	}

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