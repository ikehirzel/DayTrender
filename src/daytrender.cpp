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

namespace daytrender
{
	bool running = false;
	std::vector<Portfolio> portfolios;
	std::mutex mtx;

	bool init(const std::string& execpath)
	{
		return false;

		std::string dir = std::filesystem::current_path().string() + "/" + execpath;

		std::string clients_str = file::read(dir + "/clients.json");

		if (clients_str.empty())
		{
			FATAL("Failed to load clients.json! Aborting...");
			return false;
		}
		else
		{
			SUCCESS("SUCCESSully loaded clients.json");
		}

		std::string server_str = file::read(dir + "/server.json");

		if (server_str.empty())
		{
			FATAL("Failed to load server.json! Aborting...");
			return false;
		}
		else
		{
			SUCCESS("SUCCESSully loaded server.json");
		}

		Data server_config = Data::parse_json(server_str);
		if (server_config.is_error())
		{
			ERROR("server.json: %s", server_config.to_string());
			return false;
		}

		// if(!server::init(json, dir))
		// {
		// 	ERROR("Failed to initialize server! aborting...");
		// 	mtx.unlock();
		// 	return false;
		// }

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