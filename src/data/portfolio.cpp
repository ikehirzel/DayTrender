#include <data/portfolio.h>

// standard library
#include <cmath>

// external libraries
#include <hirzel/logger.h>
#include <hirzel/util/sys.h>
#include <api/versions.h>

using namespace hirzel;

using logger::print;

namespace daytrender
{
	Portfolio::Portfolio(const Data& config, const std::string& dir) :
		_shorting_enabled(get_shorting_enabled(config)),
		_risk_sum(get_risk_sum(config)),
		_pl(get_pl(config)),
		_risk(get_risk(config)),
		_max_loss(get_max_loss(config)),
		_history_length(get_history_length(config)),
		_closeout_buffer(get_closeout_buffer(config)),
		_assets(get_assets(config, dir)),
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

		// CLIENT	============================================================

		const Data& client_json = config["client"];
		if (!client_json.contains("filename"))
		{
			ERROR("%s: client filename must be given in config", _label);
			return;
		}

		if (!client_json.contains("keys"))
		{
			ERROR("%s: client keys must be given in config", _label);
			return;
		}

		if (!client_json["keys"].is_array())
		{
			ERROR("%s: keys were not formatted correctly", _label);
			return;
		}

		const Data& keys_json = client_json["keys"];
		_client = Client(client_json["filename"].to_string(), dir);
		if (!_client.is_bound())
		{
			ERROR("%s: client failed to bind", _label);
			return;
		}
		else
		{
			SUCCESS("%s: bound client", _label);
		}

		_client.api_version();

		// verifying api version of client matches current one
		if (_client.api_version() != CLIENT_API_VERSION)
		{
			ERROR("%s: api version for (%u) did not match current api version: %u)",
				_label, _client.api_version(), CLIENT_API_VERSION);
			return;
		}

		// incorrent number of client keys supplied
		if (_client.key_count() != keys_json.size())
		{
			ERROR("%s: expected %d keys but %d were supplied.",
				client_json["filename"].to_string(), _client.key_count(),
				keys_json.size());
			return;
		}

		// check if init failed
		const char *error = _client.init(keys_json);
		if (error)
		{
			ERROR("%s: %s", _client.filename(), error);
			return;
		}

		// setting client leverage
		//error = _client.set_leverage(config["leverage"].to_uint());
		if (error)
		{
			ERROR("%s: %s", _client.filename(), error);
			return;
		}

		// ASSETS	============================================================

		// assuring config is valid
		const Data& assets_json = config["assets"];

		if (!assets_json.is_array())
		{
			ERROR("assets must be an array");
			return;
		}

		if (assets_json.empty())
		{
			WARNING("%s: no assets to be initialized", _label);
			return;
		}


		// initializing assets
		_assets.resize(assets_json.size());
		unsigned i = 0;
		for (Asset& asset : _assets)
		{
			asset = Asset(assets_json[i], dir);
			if (!asset.is_bound())
			{
				ERROR("(%s) $%s could not be initialized", _label, _assets[i].ticker());
				_assets.clear();
				return;
			}

			i += 1;
		}

		_ok = true;
	}

	bool Portfolio::get_shorting_enabled(const Data& config) const
	{
		if (!config.contains("shorting_enabled"))
			throw std::invalid_argument("Portfolio: shorting_enabled must be defined in config");
		
		const Data& shorting_enabled = config["shorting_enabled"];

		if (!shorting_enabled.is_bool())
			throw std::invalid_argument("Portfolio: shorting_enabled must be true or false");
		
		return shorting_enabled.to_bool();
	}

	double Portfolio::get_risk(const Data& config) const
	{
		if (!config.contains("risk"))
			throw std::invalid_argument("'risk' must be defined in config'");

		const Data& risk = config["risk"];

		if (!risk.is_num())
			throw std::invalid_argument("'risk' must be a number");

		double out = risk.to_double();

		if (out < 0.0 || out > 1.0)
			throw std::out_of_range("'risk' must be in range: [0, 1]");

		return out;
	}

	double Portfolio::get_max_loss(const Data& config) const
	{
		if (!config.contains("max_loss"))
			throw std::invalid_argument("'max_loss' must be defined in config'");
		
		const Data& max_loss = config["max_loss"];

		if (!max_loss.is_num())
			throw std::invalid_argument("'max_loss' must be a number");

		double out = max_loss.to_double();

		if (out < 0.0 || out > 1.0)
			throw std::out_of_range("'max_loss' must be in range: [0, 1]");

		return out;
	}

	double Portfolio::get_history_length(const Data& config) const
	{
		if (!config.contains("history_length"))
			throw std::invalid_argument("'history_length' must be defined in config'");
		
		const Data& history_length = config["history_length"];

		if (!history_length.is_uint())
			throw std::invalid_argument("'history_length' must be a non-negative number");

		return history_length.to_uint();
	}

	unsigned Portfolio::get_closeout_buffer(const Data& config) const
	{
		if (!config.contains("closeout_buffer"))
			throw std::invalid_argument("'closeout_buffer' must be defined in config'");
		
		const Data& closeout_buffer = config["closeout_buffer"];

		if (!closeout_buffer.is_uint())
			throw std::invalid_argument("'closeout_buffer' must be a non-negative number");

		return closeout_buffer.to_uint();
	}

	Client Portfolio::get_client(const Data& config, const std::string& dir) const
	{
		if (!config.contains("client"))
			throw std::invalid_argument("Portolio: 'client' must be defined in config");
		
		return Client(config["client"], dir);
	}

	std::vector<Asset> Portfolio::get_assets(const hirzel::Data& config,
			const std::string& dir) const
	{
		if (config.contains("assets"))
			throw std::invalid_argument("'assets' must be defined in config");
		
		const Data& assets = config["assets"];

		if (!assets.is_array())
			throw std::invalid_argument("'assets' must be an array");

		std::vector<Asset> out;
		out.reserve(assets.size());

		for (const Data& asset : assets.to_array())
		{
			try
			{
				out.push_back({ asset, dir });
			}
			catch (const std::exception& e)
			{
				ERROR("failed to configure asset: %s", e.what());
				throw;
			}
		}

		return out;
	}

	void Portfolio::update()
	{
		if (!_ok)
		{
			WARNING("%s portfolio is not okay and cannot be updated", _label);
			return;
		}
		DEBUG("Updating %s portfolio information", _label);

		long long curr_time = hirzel::sys::epoch_seconds();
		_last_update = curr_time;
		// 
		//_last_update = curr_time - (curr_time % PORTFOLIO_UPDATE_INTERVAL);

		// updating pl of client
		Result<Account> res = _client.get_account();
		if (!res.ok())
		{
			ERROR("(%s) $%s: %s", _label, _client.filename(), res.error());
			return;
		}

		Account info = res.get();

		_equity_history.push_back({ curr_time, info.equity() });

		while (curr_time - _equity_history[0].first > (long long)(_history_length * 3600))
		{
			_equity_history.erase(_equity_history.begin());
		}

		double prev_equity = _equity_history.front().second;
		_pl = info.equity() - prev_equity;

		// account has lost too much in last interval
		if (_pl <= prev_equity * -_max_loss)
		{
			ERROR("%s has undergone $%f loss in the last %f hours! Closing all positions...",
				_label, _pl, _history_length);
			// shut down portfolio
			_ok = false;
			const char *error = _client.close_all_positions(_assets);
			if (error)
			{
				ERROR("%s: %s", _label, error);
				return;
			}
		}
		
		// if within closeout buffer of market is closed and there is a buffer set
		if (!is_live() && _closeout_buffer > 0)
		{
			INFO("%s market will close in %d minutes. Closing all positions...",
				_label, _closeout_buffer / 60);
			const char *error = _client.close_all_positions(_assets);
			if (error)
			{
				ERROR("%s: %s", _label, error);
				return;
			}
		}		
	}


	void Portfolio::update_assets()
	{
		DEBUG("Updating %s assets", _label);
		for (Asset& asset : _assets)
		{
			// skip if it shouldn't update yet
			if (!asset.should_update()) continue;

			Result<PriceHistory> res = _client.get_price_history(asset);
			if (!res)
			{
				// ERROR
				ERROR("(%s) $%s: %s", _label, asset.ticker(), res.error());
				continue;
			}

			unsigned action = asset.update(res.get());

			bool update_portfolio = false;

			switch (action)
			{
			case ENTER_LONG:
				_client.enter_long(asset, _risk / risk_sum());
				update_portfolio = true;
				break;

			case EXIT_LONG:
				_client.exit_long(asset);
				update_portfolio = true;
				break;

			case ENTER_SHORT:
				_client.enter_short(asset, _risk / risk_sum());
				update_portfolio = true;
				break;

			case EXIT_SHORT:
				_client.exit_short(asset);
				update_portfolio = true;
				break;

			case NOTHING:
				INFO("(%s) $%s: No action taken", _label, asset.ticker());
				break;

			case ERROR:
				ERROR("(%s) $%s: failed to update", _label, asset.ticker());
				_ok = false;
				break;

			default:
				ERROR("(%s) $%s: Invalid action received from strategy: %d",
					_label, asset.ticker(), action);
				break;
			}

			// if an order was placed
			if (update_portfolio) update();
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


	Asset *Portfolio::get_asset(const std::string& ticker)
	{
		for (Asset& a : _assets)
		{
			if (a.ticker() == ticker) return &a;
		}
		return nullptr;
	}
}