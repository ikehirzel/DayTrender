#include "daytrender.h"

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

#include "api/portfolio.h"
#include "interface/interface.h"
#include "interface/shell.h"
#include "interface/server.h"
#include "util/jsonutil.h"

#include <filesystem>
#include <thread>
#include <mutex>

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

		std::string clients_str = file::read_file_as_string(dir + "/clients.json");

		if (clients_str.empty())
		{
			FATAL("Failed to load clients.json! Aborting...");
			return false;
		}
		else
		{
			SUCCESS("SUCCESSully loaded clients.json");
		}

		std::string server_str = file::read_file_as_string(dir + "/server.json");

		if (server_str.empty())
		{
			FATAL("Failed to load server.json! Aborting...");
			return false;
		}
		else
		{
			SUCCESS("SUCCESSully loaded server.json");
		}

		JsonValue json_val;
		std::string err = picojson::parse(json_val, server_str);

		if (!err.empty())
		{
			ERROR("Portfolio config: %s", err);
			return false;
		}

		if(!server::init(json_val.get<JsonObject>(), dir))
		{
			ERROR("Failed to initialize server! aborting...");
			mtx.unlock();
			return false;
		}

		return true;
		
		///////////////////////////////////////////////

		// std::cout << "Getting account\n";
		// auto acct = clients[0]->get_account_info();
		// printfmt("Balance: %f\nBuying_power: %f\nEquity: %f\nLeverage: %d\n\n", acct.balance(), acct.buying_power(), acct.equity(), acct.leverage());

		// clients[0]->enter_long("EUR_USD", 0.1);

		// sys::thread_sleep(1000);
		// std::cout << "Getting account\n";
		// acct = clients[0]->get_account_info();
		// printfmt("Balance: %f\nBuying_power: %f\nEquity: %f\nLeverage: %d\n\n", acct.balance(), acct.buying_power(), acct.equity(), acct.leverage());

		// //std::cout << "Exiting\n";
		// clients[0]->exit_long("EUR_USD");

		// sys::thread_sleep(1000);
		// std::cout << "Getting account\n";
		// acct = clients[0]->get_account_info();
		// printfmt("Balance: %f\nBuying_power: %f\nEquity: %f\nLeverage: %d\n\n", acct.balance(), acct.buying_power(), acct.equity(), acct.leverage());


		////////////////////////////////////////////

		// PaperAccount acc(500.0, 10, 0.0, 1.0, 2.0, true, 300, { 1, 2, 3 });
		// acc.handle_action(Action::ENTER_SHORT);
		// std::cout << "Enter long " << acc << std::endl;
		// acc.update_price(1.5);
		// std::cout << "Update " << acc << std::endl;
		// acc.handle_action(Action::EXIT_SHORT);
		// std::cout << "Exit short " << acc << std::endl;
		// std::cout << "Error: " << acc.error() << std::endl;

		////////////////////////////////////////////

		// auto res = interface::backtest(0, 0, 500, true, 5, 155, 10, { });
		// for (int i = 0; i < res.size(); i++)
		// {
		// 	std::cout << "Account " << i+1 << ' ' << res[i] << std::endl;
		// 	//break;
			
		// }
		// std::cout << "\n\n";
		// assets[0]->update();
		// std::cout << "\n\n";
	}

	void start()
	{
		running = true;
		INFO("Starting DayTrender");

		std::thread shell_thread(shell::get_input);
		shell_thread.detach();

		std::thread server_thread(server::start);

		while (running)
		{
			for (Portfolio& portfolio : portfolios)
			{
				portfolio.update();
			}
			
			sys::thread_sleep(1000);
		}

		server::stop();
		server_thread.join();
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
	hirzel::fountain::init("./report.log", true, true, false, 1);

	if (!daytrender::init(hirzel::str::get_folder(argv[0])))
	{
		ERROR("Execution cannot continue");
		return 0;
	}

	daytrender::start();
	daytrender::Strategy::free_plugins();

	SUCCESS("DayTrender has stopped");
	return 0;
}