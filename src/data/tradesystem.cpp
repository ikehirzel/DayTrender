#include <data/tradesystem.h>

// local inlcudes
#include <interface/backtest.h>
#include <interface/shell.h>
//#include <interface/server.h>

// standard libararies
#include <filesystem>
#include <thread>
#include <mutex>

// external libraries
#include <hirzel/logger.h>
#include <hirzel/util/file.h>
#include <hirzel/util/str.h>
#include <hirzel/util/sys.h>

using namespace hirzel;

#define CONFIG_FOLDER "/config"

namespace daytrender
{
	TradeSystem::TradeSystem(const std::string& dir)
	{
		_initialized = init(dir);
		if (!_initialized) _portfolios.clear();
	}

	bool TradeSystem::init(const std::string& dir)
	{
		
		std::string portfolios_str = file::read(dir + CONFIG_FOLDER "/portfolios.json");

		if (portfolios_str.empty())
		{
			FATAL("failed to read portfolios.json");
			return false;
		}

		Data portfolios_json = Data::parse_json(portfolios_str);
		if (portfolios_json.is_error())
		{
			FATAL("portfolios.json: %s", portfolios_json.to_string());
			return false;
		}

		if (!portfolios_json.is_table())
		{
			FATAL("portfolios.json is not the correct format");
			return false;
		}

		SUCCESS("Loaded porfolios.json");

		// reading portfolio configs
		const Data::Table& table = portfolios_json.to_table();
		for (auto pair : table)
		{
			const std::string& label = pair.first;
			std::string filename = pair.second.to_string();
			
			std::string file = file::read(dir + CONFIG_FOLDER "/portfolios/" + filename);
			if (file.empty())
			{
				ERROR("Failed to load config for %s portfolio: %s", label, filename);
				return false;
			}
			
			Data json = Data::parse_json(file);
			if (json.is_error())
			{
				ERROR("%s: %s", pair.second.to_string(), json.to_string());
				return false;
			}

			SUCCESS("Loaded %s", filename);

			Portfolio portfolio(json, label, dir);
			if (portfolio.is_ok())
			{
				_portfolios.push_back(portfolio);
			}
		}

		if (_portfolios.empty())
		{
			WARNING("No portfolios were loaded...");
			return false;
		}

		return true;
	}

	void TradeSystem::start()
	{
		_running = true;
		INFO("Starting DayTrender");

		while (_running)
		{
			for (Portfolio& portfolio : _portfolios)
			{
				// do nothing if portfolio is not live
				if (!portfolio.is_live()) continue;
				// update account/ pl info if hasn't been done recently
				if (portfolio.should_update()) portfolio.update();
				// update assets
				portfolio.update_assets();
			}
			
			// check all portfolios every 3 seconds
			sys::sleep_millis(3000);
		}
	}

	void TradeSystem::stop()
	{
		_mtx.lock();
		if (!_running)
		{
			WARNING("DayTrender has already stopped");
			return;
		}

		_running = false;
		INFO("Shutting down DayTrender...");
		_mtx.unlock();
	}

	Portfolio *TradeSystem::get_portfolio(const std::string& label)
	{
		for (Portfolio& p : _portfolios)
		{
			if (p.label() == label) return &p;
		}
		return nullptr;
	}
}
