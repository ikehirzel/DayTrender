#include "portfolio.h"

#include <hirzel/logger.h>

using namespace hirzel;

namespace daytrender
{
	const std::vector<const char*> check_array = { "assets" };
	const std::vector<const char*> check_ratio = { "max_loss", "risk" };
	const std::vector<const char*> check_positive =
	{
		"history_length",
		"leverage",
		"closeout_buffer"
	};

	Portfolio::Portfolio(const std::string& label, const Data& config, const std::string& dir) :
	_label(label)
	{
		double max_loss = config["max_loss"].to_double();
		double risk = config["risk"].to_double();
		if (max_loss <= 0.0 || max_loss > 1.0)
		{
			ERROR("max loss must be a ratio");
			return;
		}

		if (risk <= 0.0 || risk > 1.0)
		{
			ERROR("risk must be a ratio");
			return;
		}

		if (!config["assets"].is_array())
		{
			ERROR("assets must be an array");
			return;
		}
		if (!json_vars_are_arrays(config, check_array)) return;
		if (!json_vars_are_positive(config, check_positive)) return;
	
		_client = Client(config.get("client"), dir);

		const picojson::array& assets_json = config.get("assets").get<picojson::array>();
		_assets.resize(assets_json.size());
		for (int i = 0; i < assets_json.size(); i++)
		{
			Asset a(assets_json[i], dir);
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