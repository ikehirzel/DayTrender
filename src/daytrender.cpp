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
	bool shouldrun = false;
	std::string dtdir;
	std::vector<Portfolio> portfolios;
	std::mutex mtx;

	void init(const std::string& execpath)
	{
		shouldrun = false;
		return;

		dtdir = std::filesystem::current_path().string() + "/" + execpath;

		std::string clients_str = file::read_file_as_string(dtdir + "/clients.json");

		if (clients_str.empty())
		{
			FATAL("Failed to load clients.json! Aborting...");
			return;
		}
		else
		{
			SUCCESS("SUCCESSully loaded clients.json");
		}

		std::string server_str = file::read_file_as_string(dtdir + "/server.json");

		if (server_str.empty())
		{
			FATAL("Failed to load server.json! Aborting...");
			return;
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
			return;
		}

		if(!server::init(json_val.get<JsonObject>(), dtdir))
		{
			ERROR("Failed to initialize server! aborting...");
			mtx.unlock();
			return;
		}

		shouldrun = true;
		
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

	// Asset init_asset(const json& config, int index)
	// {
	// 	// making sure the necessary variables are defined in the json file
	// 	if (!json_vars_are_defined(config, {
	// 		"ticker",
	// 		"strategy",
	// 		"interval",
	// 		"ranges"
	// 	})) return {};


	// 	// getting interval from json
	// 	int interval = config["interval"].get<int>();

	// 	// getting the index of the strategy
	// 	Strategy* strat = init_strategy(config["strategy"].get<std::string>());
	// 	// verifying if the strategy initialized
	// 	if (strat == nullptr)
	// 	{
	// 		ERROR("clients.json[%d]: Asset cannot be initialized without strategy", index);
	// 		return {};
	// 	}

	// 	// getting ranges from json
	// 	std::vector<int> ranges(config["ranges"].begin(), config["ranges"].end());

	// 	// verifying that the correct amount of ranges were given
	// 	if (ranges.size() != strat->indicator_count())
	// 	{
	// 		ERROR("clients.json[%d]: Expected %d ranges but %d were supplied.", index,
	// 			strat->indicator_count(), ranges.size());

	// 		return {};
	// 	}

	// 	// verifying that every range given is a positive number
	// 	for (int r : ranges)
	// 	{
	// 		if (r <= 0)
	// 		{
	// 			ERROR("asset[%d]: 'ranges' were improperly defined. Values must be a positive number.", index);
	// 			return {};
	// 		}
	// 	}

	// 	// getting ticker from json
	// 	std::string ticker = config["ticker"].get<std::string>();
	// 	// verifying ticker was non empty
	// 	if (ticker.empty())
	// 	{
	// 		ERROR("clients.json[%d][%d]: 'ticker' definition was empty.", index, ticker);
	// 		return {};
	// 	}

	// 	Asset asset(strat, ticker, interval, ranges);
	// 	SUCCESS("%s: SUCCESSully initialized with risk: %f", ticker, asset.risk());
	// 	return asset;
	// }

	/**
	 * Initializes server and shell and begins updating assets
	 */
	void start()
	{
		mtx.lock();

		if (running)
		{
			WARNING("DayTrender has already started!");
			mtx.unlock();
			return;
		}

		if (!shouldrun)
		{
			FATAL("Execution of DayTrender cannot continue!");
			mtx.unlock();
			return;
		}

		running = true;
		mtx.unlock();

		std::thread shell_thread(shell::get_input);
		std::thread server_thread(server::start);
		shell_thread.detach();
		INFO("Starting DayTrender");

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

	/**
	 * Changes running to false. This will interrupt the program loop.
	 */
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

	/**
	 * @return list of names corresponding to loaded clients
	 */
	std::vector<std::string> portfolio_names()
	{
		std::vector<std::string> out(portfolios.size());

		for (int i = 0; i < portfolios.size(); i++)
		{
			out[i] = portfolios[i].label();
		}

		return out;
	}


	/**
	 * @param	index	index of the portfolio
	 * @return	reference to portfolio at \c index
	 */
	const Portfolio& get_portfolio(int index)
	{
		return portfolios[index];
	}
}

int main(int argc, char *argv[])
{
	hirzel::fountain::init("./report.log", true, true, false, 1);
	daytrender::init(hirzel::str::get_folder(argv[0]));
	daytrender::start();
	daytrender::Strategy::free_plugins();
	SUCCESS("DayTrender has stopped");
	return 0;
}