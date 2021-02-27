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
	bool shouldrun = true;
	std::string dtdir;

	// data containers
	std::vector<Portfolio> portfolios;
	std::vector<Strategy*> strategies;

	std::mutex mtx;

	Portfolio init_portfolio(const json& config, int index);
	Client init_client(const json& config, int index);
	Asset init_asset(const json& config, int type, int index);
	Strategy* init_strategy(const std::string& filename);

	void init(const std::string& execpath)
	{
		mtx.lock();

		dtdir = std::filesystem::current_path().string() + "/" + execpath;

		std::string clients_str = file::read_file_as_string(dtdir + "/clients.json");

		if (clients_str.empty())
		{
			FATAL("Failed to load clients.json! Aborting...");
			shouldrun = false;
			mtx.unlock();
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
			shouldrun = false;
			mtx.unlock();
			return;
		}
		else
		{
			SUCCESS("SUCCESSully loaded server.json");
		}

		INFO("Initializing clients...");
		json clients_json = json::parse(clients_str);

		for (int i = 0; i < clients_json.size(); i++)
		{
			Client cli = init_client(clients_json[i], i);
			if (!cli.is_live())
			{
				shouldrun = false;
				return;
			}
		}

		INFO("Initialized portfolios:	%d", portfolios.size());
		INFO("Initialized strategies:  %d", strategies.size());

		json server_json = json::parse(server_str);

		if(!server::init(server_json, dtdir))
		{
			ERROR("Failed to initialize server! aborting...");
			shouldrun = false;
		}

		mtx.unlock();
		shouldrun = false;
		
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

	Portfolio init_portfolio(const json& config, int index)
	{
		// INFO("%s: Initializing %s assets...", cli->filename(), cli->label());
		// // allocating all the assets for the client;
		// const json& assets_json = config["assets"];
		// for (int i = 0; i < assets_json.size(); i++)
		// {
		// 	if (!init_asset(assets_json[i], index, i)) return false;
		// }
		return {};
	}

	Client init_client(const json& config, int index)
	{
		// checking if vars are defined
		if (!json_vars_are_defined(config, {
			"label",
			"filename",
			"shorting_enabled",
			"risk",
			"max_loss",
			"history_length",
			"leverage",
			"closeout_buffer",
			"assets"
		})) return {};

		// checking if vars are between 0 and 1
		if (!json_vars_are_ratios(config, {
			"max_loss",
			"risk",
		})) return {};

		// checking if vars are at least 0
		if (!json_vars_are_positive(config, {
			"history_length",
			"leverage",
			"closeout_buffer"
		})) return {};

		Client cli(config, dtdir + CLIENT_DIR);

		if (!cli.bound())
		{
			ERROR("%s: Client failed to bind. Assets will not be loaded.", cli.filename());
			return {};
		}

		SUCCESS("SUCCESSully initialized client: '%s'", cli.filename());
		return cli;
	}

	Asset init_asset(const json& config, int index)
	{
		// making sure the necessary variables are defined in the json file
		if (!json_vars_are_defined(config, {
			"ticker",
			"strategy",
			"interval",
			"ranges"
		})) return {};


		// getting interval from json
		int interval = config["interval"].get<int>();

		// getting the index of the strategy
		Strategy* strat = init_strategy(config["strategy"].get<std::string>());
		// verifying if the strategy initialized
		if (strat == nullptr)
		{
			ERROR("clients.json[%d]: Asset cannot be initialized without strategy", index);
			return {};
		}

		// getting ranges from json
		std::vector<int> ranges(config["ranges"].begin(), config["ranges"].end());

		// verifying that the correct amount of ranges were given
		if (ranges.size() != strat->indicator_count())
		{
			ERROR("clients.json[%d]: Expected %d ranges but %d were supplied.", index,
				strat->indicator_count(), ranges.size());

			return {};
		}

		// verifying that every range given is a positive number
		for (int r : ranges)
		{
			if (r <= 0)
			{
				ERROR("asset[%d]: 'ranges' were improperly defined. Values must be a positive number.", index);
				return {};
			}
		}

		// getting ticker from json
		std::string ticker = config["ticker"].get<std::string>();
		// verifying ticker was non empty
		if (ticker.empty())
		{
			ERROR("clients.json[%d][%d]: 'ticker' definition was empty.", index, ticker);
			return {};
		}

		Asset asset(strat, ticker, interval, ranges);
		SUCCESS("%s: SUCCESSully initialized with risk: %f", ticker, asset.risk());
		return asset;
	}
	
	/**
	 * @param	filename	filename of the strategy to use including extension
	 * @return				pointer to the strategy corresponding to filename
	 */
	Strategy* init_strategy(const std::string& filename)
	{
		for (int i = 0; i < strategies.size(); i++)
		{
			if (strategies[i]->filename() == filename)
			{
				return strategies[i];
			}
		}

		Strategy* strat = new Strategy(dtdir + STRATEGY_DIR + filename);

		if (strat->is_bound())
		{
			strategies.push_back(strat);
			SUCCESS("SUCCESSully initialized strategy: '%s'", filename);
			return strat;
		}
		else
		{
			ERROR("Strategy '%s' did not bind correctly", filename);
			delete strat;
			return nullptr;
		}
	}

	/**
	 * Frees all of the dynamically allocated memory used by daytrender.
	 * When deleting clients it closes all positions.
	 */
	void free()
	{
		mtx.lock();

		if (strategies.empty())
		{
			WARNING("No strategies were initialized");
		}
		else
		{
			INFO("Destructing strategies...");
			for (int i = 0; i < strategies.size(); i++)
			{
				delete strategies[i];
			}
		}

		mtx.unlock();
	}

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
	 * @return	bool representing running state of daytrender
	 */
	bool is_running()
	{
		return running;
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

	const Portfolio& get_portfolio(int index) { return portfolios[index]; }
}

int main(int argc, char *argv[])
{
	hirzel::fountain::init("./report.log", true, true, false, 1);
	daytrender::init(hirzel::str::get_folder(argv[0]));
	daytrender::start();
	daytrender::free();
	SUCCESS("DayTrender has stopped");
	return 0;
}