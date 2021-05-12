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
	const std::vector<const char*> check_array = { "assets" };
	const std::vector<const char*> check_ratio = { "max_loss", "risk" };
	const std::vector<const char*> check_positive =
	{
		"history_length",
		"leverage",
		"closeout_buffer"
	};

	Portfolio::Portfolio(const Data& config, const std::string& label,
		const std::string& dir) :
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
		
		//int history_length = config["h"

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

		// get asset information
		const Data& assets_json = config["assets"];
		if (!assets_json.is_array()) return;

		// initializing assets
		_assets.resize(assets_json.size());
		for (int i = 0; i < assets_json.size(); i++)
		{
			_assets[i] = Asset(assets_json[i], dir);
			if (!_assets[i].is_bound())
			{
				ERROR("%s (%s) could not be initialized", _assets[i].ticker(), _label);
				_assets.clear();
				return;
			}
		}

		_live = true;
	}


	void Portfolio::update()
	{
		INFO("Updating %s", _label);
		unsigned till_close = _client.secs_till_market_close();

		if (till_close <= _closeout_buffer) return;
	
		// updating pl of client
		Result<Account> res = _client.get_account();
		if (!res.ok())
		{
			ERROR("%s (%s): %s", _client.filename(), _label, res.error());
			return;
		}

		Account info = res.get();

		long long curr_time = hirzel::sys::epoch_seconds();
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
			ERROR("%s has undergone $%f loss in the last %f hours! Closing all position...", _label, _pl, _history_length);
			
			const char *error = _client.close_all_positions();
			if (error)
			{
				ERROR("%s (%s): %s", _client.filename(), _label, error);
			}

			INFO("%s trading has ceased!", _label);
			return;
		}

		// checking to see if in range of closeout buffer
		
		if (till_close <= _closeout_buffer)
		{
			INFO("%s market will close in %d minutes. Closing all positions...", _label, _closeout_buffer / 60);
			const char *error = _client.close_all_positions();
			if (error)
			{
				ERROR("%s (%s): %s", _client.filename(), _label, error);

			}
		}


		for (Asset& asset : _assets)
		{
			unsigned action = asset.update(_client);


			bool res = false;
			switch (action)
			{
			case ENTER_LONG:
				enter_position(asset, false);
				break;

			case EXIT_LONG:
				exit_position(asset, false);
				break;

			case ENTER_SHORT:
				enter_position(asset, true);
				break;

			case EXIT_SHORT:
				exit_position(asset, true);
				break;

			case NOTHING:
				INFO("%s (%s): No action taken", asset.ticker(), _label);
				break;

			case ERROR:
				ERROR("%s (%s): failed to update", asset.ticker(), _label);
				break;

			default:
				ERROR("%s (%s): Invalid action received from strategy: %d",
					asset.ticker(), _label, action);
				break;
			}
		}
	}

	/**
	 * @brief	Orders max amount of shares orderable
	 * 
	 * @param	ticker			the symbol that will be evalutated
	 * @param	pct				the percentage of available buying power to use
	 * @param	short_shares	true if going short shares, false if going long on shares
	 * 
	 * @return	the amount of shares to order
	 */

	void Portfolio::enter_position(const Asset& asset, bool short_shares)
	{
		// if not buying anything, exit
		if (asset.risk() == 0.0) return;

		// will be -1.0 if short_shares is true or 1.0 if it's false
		double multiplier = (double)short_shares * -2.0 + 1.0;

		// getting current account information
		Result<Account> acc_res = _client.get_account();

		if (!acc_res.ok())
		{
			ERROR("%s: %s", _label, acc_res.error());
			return;
		}

		Account acc_info = acc_res.get();

		Result<Position> asset_res = _client.get_position(asset.ticker());

		if (!asset_res.ok())
		{
			ERROR("%s (%s): %s", asset.ticker(), _label, asset_res.error());
			return;
		}

		// getting position info
		Position asset_info = asset_res.get();

		// base buying power
		double buying_power = (acc_info.buying_power() + acc_info.margin_used())
			* _risk * (asset.risk() / risk_sum());

		// if we are already in a position of the same type as requested
		if (asset_info.shares() * multiplier > 0.0)
		{
			// remove the current share of the buying power
			buying_power -= asset_info.amt_invested();
		}
		// we are in a position that is opposite to type requested
		else if (asset_info.shares() * multiplier < 0.0)
		{
			// calculate returns upon exiting position for correct buying power calculation
			buying_power += asset_info.shares() * asset_info.price() * (1.0 - multiplier * asset_info.fee());
		}

		double shares = multiplier * std::floor(((buying_power / (1.0 + asset_info.fee())) / asset_info.price()) / asset_info.minimum()) * asset_info.minimum();
		INFO("Placing order for %f shares!!!", shares);
		
		_client.market_order(asset.ticker(), shares);
	}

	void Portfolio::exit_position(const Asset& asset, bool short_shares)
	{
		double multiplier = (double)short_shares * -2.0 + 1.0;
		Result<Position> asset_res = _client.get_position(asset.ticker());
		if (!asset_res.ok())
		{
			ERROR("%s (%s): %s", asset.ticker(), _label, asset_res.error());
			return;
		}

		Position info = asset_res.get();

		// if we are in a short position or have no shares, do nothing
		if (info.shares() * multiplier <= 0.0) return;

		// exit position
		if (!_client.market_order(asset.ticker(), -info.shares()))
		{
			ERROR("%s (%s): Failed to exit position of %f shares",
				asset.ticker(), _label, info.shares());
		}
		else
		{
			SUCCESS("%s (%s): Successfully exited position of %f shares",
				asset.ticker(), _label, info.shares());
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