#include <daytrender.h>

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
	bool running = false;
	std::vector<Portfolio> portfolios;
	std::mutex mtx;

	bool init(const std::string& execpath)
	{
		std::string dir = std::filesystem::current_path().string() + "/" + execpath;
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

		// reading portfolio configs
		const Data::Table& table = portfolios_json.to_table();
		for (auto pair : table)
		{
			const std::string& label = pair.first;
			INFO("Loading %s portfolio", label);
			std::string file = file::read(dir + CONFIG_FOLDER "/portfolios/" + pair.second.to_string());
			Data json = Data::parse_json(file);
			if (json.is_error())
			{
				ERROR("%s: %s", pair.second.to_string(), json.to_string());
				return false;
			}

			Portfolio portfolio(json, label, dir);
			if (portfolio.is_live())
			{
				portfolios.push_back(portfolio);
			}
		}

		if (portfolios.empty())
		{
			WARNING("No portfolios were loaded...");
			return false;
		}

		return true;
	}

	void start()
	{
		running = true;
		INFO("Starting DayTrender");

		// std::thread shell_thread(shell::get_input);
		// shell_thread.detach();

		// std::thread server_thread(server::start);

		while (running)
		{
			for (Portfolio& portfolio : portfolios)
			{
				portfolio.update();
			}
			
			sys::sleep_millis(1000);
		}

		// server::stop();
		// server_thread.join();
	}

	void stop()
	{
		mtx.lock();
		if (!running)
		{
			WARNING("DayTrender has already stopped");
			return;
		}

		running = false;
		INFO("Shutting down DayTrender...");
		mtx.unlock();
	}

	const std::vector<Portfolio>& get_portfolios()
	{
		return portfolios; 
	}
}

int main(int argc, char *argv[])
{
	hirzel::logger::init();

	// 
	if (!daytrender::init(hirzel::str::get_folder(argv[0])))
	{
		FATAL("DayTrender failed to initialize");
		return 0;
	}

	daytrender::start();
	daytrender::Strategy::free_plugins();

	SUCCESS("DayTrender has stopped");

	return 0;
}