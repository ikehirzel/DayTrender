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
			if (portfolio.is_live())
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

		// std::thread shell_thread(shell::get_input);
		// shell_thread.detach();

		// std::thread server_thread(server::start);

		while (_running)
		{
			for (Portfolio& portfolio : _portfolios)
			{
				portfolio.update();
			}
			
			sys::sleep_millis(1000);
		}

		// server::stop();
		// server_thread.join();
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
}
