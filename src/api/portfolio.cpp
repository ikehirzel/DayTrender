#include "portfolio.h"

// standard library
#include <cmath>

// external libraries
#include <hirzel/logger.h>
#include <hirzel/util/sys.h>

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

	Portfolio::Portfolio(const Data& config, const std::string& dir) :
	_label(config["label"].to_string())
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
	
		_client = Client(config["client"].to_string(), dir);

		const picojson::array& assets_json = config.get("assets").get<picojson::array>();
		_assets.resize(assets_json.size());
		for (int i = 0; i < assets_json.size(); i++)
		{
			Asset a(assets_json[i], dir);
			if (!a.is_live()) return;
			_assets[i] = a;
		}

		
		// verifying api version of client matches current one
		int api_version = _api_version();
		if (api_version != CLIENT_API_VERSION)
		{
			ERROR("%s: api version for (%d) did not match current api version: %d)", api_version, CLIENT_API_VERSION);
			return;
		}

		// verifying that the correct amount of credentials passed
		int key_count = _key_count();
		if (key_count != keys.size())
		{
			ERROR("%s: expected %d keys but %d were supplied.", _filename, key_count, keys.size());
			return;
		}
		
		// initializing the client
		if (!_init(keys))
		{
			flag_error();
			return;
		}

		// setting client leverage
		if (!_set_leverage(leverage))
		{
			flag_error();
			return;
		}

		_live = true;
	}


	void Portfolio::update()
	{
		int till_close = _client.secs_till_market_close();

		if (_live)
		{
			// updating pl of client
			AccountInfo info = _client.get_account_info();
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
				ERROR("client '%s' has undergone %f loss in the last %f hours! Closing all position...", _filename, _pl, _history_length);
				close_all_positions();
				_bound = false;
				ERROR("client '%s' has gone offline!", _filename);
				return;
			}

			// checking to see if in range of closeout buffer
			
			if (till_close <= _closeout_buffer)
			{
				INFO("Client '%s': Market will close in %d minutes. Closing all positions...", _filename, _closeout_buffer / 60);
				_live = false;
				if (!close_all_positions())
				{
					ERROR("Client '%s': Failed to close positions!", _filename);
				}
			}
		}
		else
		{
			if (till_close > 0)
			{
				_live = true;
			}
		}



		/****
		 * ASSET UPATE CODE
		 *////

		bool res = false;
		switch (_data.action())
		{
		case ENTER_LONG:
			res = client.enter_long(_ticker, _risk);
			break;

		case EXIT_LONG:
			res = client.exit_long(_ticker);
			SUCCESS("%s: Exited long position", _ticker);
			break;

		case ENTER_SHORT:
			res = client.enter_short(_ticker, _risk);
			SUCCESS("%s: Entered short position", _ticker);
			break;

		case EXIT_SHORT:
			res = client.exit_short(_ticker);
			SUCCESS("%s: Exited short position", _ticker);
			break;

		case NOTHING:
			res = true;
			SUCCESS("%s: No action taken", _ticker);
			break;

		default:
			ERROR("%s: Invalid action received from strategy: %d", _ticker, _data.action());
			_live = false;
			break;
		}

		if (!res)
		{
			_live = false;
			ERROR("%s: Failed to handle action. Asset is no longer live.", _ticker);
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

	void Portfolio::enter_position(const std::string& ticker)
	{
		///
		//	REMOVE THESE LATER
		double asset_risk = 0.0;
		bool short_shares = false;
		////////////////////////



		// if not buying anything, exit
		if (asset_risk == 0.0) return;

		// will be -1.0 if short_shares is true or 1.0 if it's false
		double multiplier = (double)short_shares * -2.0 + 1.0;

		// getting current account information
		AccountInfo account = _client.get_account_info();
		// getting position info
		AssetInfo asset = _client.get_asset_info(ticker);

		// base buying power
		double buying_power = (account.buying_power() + account.margin_used()) * _risk
			* (asset_risk / _risk_sum);

		// if we are already in a position of the same type as requested
		if (asset.shares() * multiplier > 0.0)
		{
			// remove the current share of the buying power
			buying_power -= asset.amt_invested();
		}
		// we are in a position that is opposite to type requested
		else if (asset.shares() * multiplier < 0.0)
		{
			// calculate returns upon exiting position for correct buying power calculation
			buying_power += asset.shares() * asset.price() * (1.0 - multiplier * asset.fee());
		}

		double shares = multiplier * std::floor(((buying_power / (1.0 + asset.fee())) / asset.price()) / asset.minimum()) * asset.minimum();
		INFO("Placing order for %f shares!!!", shares);
		
		_client.market_order(ticker, shares);
	}

	void Portfolio::exit_position(const std::string& ticker)
	{
		///
		// REMOVE THESE LATER
		bool short_shares = false;
		//////////////////////

		double multiplier = (double)short_shares * -2.0 + 1.0;
		AssetInfo info = _client.get_asset_info(ticker);
		// if we are in a short position or have no shares, do nothing
		INFO("%s: Exiting position of %f shares", ticker, info.shares());
		if (info.shares() * multiplier <= 0.0) return;
		// exit position
		_client.market_order(ticker, -info.shares());
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