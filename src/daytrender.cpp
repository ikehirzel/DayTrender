#include "daytrender.h"

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

#include "interface/interface.h"
#include "interface/shell.h"
#include "interface/server.h"

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
	std::vector<Asset*> assets;
	std::vector<Strategy*> strategies;
	std::vector<Client*> clients;

	std::mutex mtx;

	bool init_client(const json& config, int index);
	bool init_asset(const json& config, int type, int index);
	int init_strategy(const std::string& filename);

	void init(const std::string& execpath)
	{
		mtx.lock();

		dtdir = std::filesystem::current_path().string() + "/" + execpath;

		std::string clients_str = file::read_file_as_string(dtdir + "/clients.json");

		if (clients_str.empty())
		{
			fatalf("Failed to load clients.json! Aborting...");
			shouldrun = false;
			mtx.unlock();
			return;
		}
		else
		{
			successf("Successfully loaded clients.json");
		}

		std::string server_str = file::read_file_as_string(dtdir + "/server.json");

		if (server_str.empty())
		{
			fatalf("Failed to load server.json! Aborting...");
			shouldrun = false;
			mtx.unlock();
			return;
		}
		else
		{
			successf("Successfully loaded server.json");
		}

		infof("Initializing clients...");
		json clients_json = json::parse(clients_str);

		for (int i = 0; i < clients_json.size(); i++)
		{
			// initialize client. if it fails, stop daytrender from running
			if (!init_client(clients_json[i], i)) shouldrun = false;
		}

		infof("Initialized clients:       %d", clients.size());
		infof("Initialized assets:        %d", assets.size());
		infof("Initialized strategies:    %d", strategies.size());

		json server_json = json::parse(server_str);

		if(!server::init(server_json, dtdir))
		{
			errorf("Failed to initialize server! aborting...");
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

	bool check_is_defined(const std::string& name, const json& config, const std::vector<std::string>& var_names)
	{
		for (int i = 0; i < var_names.size(); i++)
		{
			if (config.find(var_names[i]) == config.end())
			{
				errorf("%s: '%s' was not defined", name, var_names[i]);
				return false;
			}
		}
		return true;
	}

	bool check_is_in_range(const std::string& name, const json& config, double min, double max, const std::vector<std::string>& var_names)
	{
		for (int i = 0; i < var_names.size(); i++)
		{
			double val = config[var_names[i]].get<double>();
			// if max is set to min, it'll just check for min
			if (val > max && min < max)
			{
				errorf("%s: '%s' was above maximum: %f", name, var_names[i], max);
				return false;
			}
			else if (val < min)
			{
				errorf("%s: '%s' was below minimum: %f", name, var_names[i], min);
				return false;
			}
		}
		return true;
	}

	bool init_client(const json& config, int index)
	{
		// checking if vars are defined
		if (!check_is_defined("clients.json[" + std::to_string(index) + "]", config, {
			"label",
			"filename",
			"shorting_enabled",
			"risk",
			"max_loss",
			"history_length",
			"leverage",
			"closeout_buffer",
			"assets"
		})) return false;

		// checking if vars are between 0 and 1
		if (!check_is_in_range("clients.json", config, 0, 1.0, {
			"max_loss",
			"risk",
		})) return false;

		// checking if vars are at least 0
		if (!check_is_in_range("clients.json", config, 0, 0, {
			"history_length",
			"leverage",
			"closeout_buffer"
		})) return false;

		Client* cli = new Client(config, dtdir + CLIENT_DIR);

		if (!cli->bound())
		{
			errorf("%s: Client failed to bind. Assets will not be loaded.", cli->filename());
			delete cli;
			return false;
		}

		clients.push_back(cli);

		infof("%s: Initializing %s assets...", cli->filename(), cli->label());
		// allocating all the assets for the client;
		const json& assets_json = config["assets"];
		for (int i = 0; i < assets_json.size(); i++)
		{
			if (!init_asset(assets_json[i], index, i)) return false;
		}
		successf("Successfully initialized %s client: '%s'", cli->label(), cli->filename());
		return true;
	}

	bool init_asset(const json& config, int type, int index)
	{
		// making sure the necessary variables are defined in the json file
		if (!check_is_defined("clients.json[" + std::to_string(type) +"][" + std::to_string(index) + "]", config, {
			"ticker",
			"strategy",
			"interval",
			"ranges"
		})) return false;


		// getting interval from json
		int interval = config["interval"].get<int>();
		// verifying that the given interval is one supported by the client
		if (clients[type]->to_interval(interval) == "")
		{
			errorf("clients.json[%d][%d]: 'interval' was improperly defined", type, index);
			return false;
		}

		// getting the index of the strategy
		int strat_index = init_strategy(config["strategy"].get<std::string>());
		// verifying if the strategy initialized
		if (strat_index < 0)
		{
			errorf("clients.json[%d][%d]: Asset cannot be initialized without strategy", type, index);
			return false;
		}

		// getting ranges from json
		std::vector<int> ranges(config["ranges"].begin(), config["ranges"].end());

		// verifying that the correct amount of ranges were given
		if (ranges.size() != strategies[strat_index]->indicator_count())
		{
			errorf("clients.json[%d][%d]: Expected %d ranges but %d were supplied.", type, index,
				strategies[strat_index]->indicator_count(), ranges.size());

			return false;
		}

		// verifying that every range given is a positive number
		for (int r : ranges)
		{
			if (r <= 0)
			{
				errorf("clients.json[%d][%d]: 'ranges' were improperly defined. Values must be a positive number.", type, index);
				return false;
			}
		}

		// getting ticker from json
		std::string ticker = config["ticker"].get<std::string>();
		// verifying ticker was non empty
		if (ticker.empty())
		{
			errorf("clients.json[%d][%d]: 'ticker' definition was empty.", type, index, ticker);
			return false;
		}

		assets.push_back(new Asset(clients[type], strategies[strat_index], ticker, type, interval, ranges));	
		successf("Successfully initialized asset: '%s'", ticker);
		return true;
	}
	
	/**
	 * Returns the index of the strategy with the filename given or -1 if it failed to initialize
	 * @param	filename	filename of the strategy to use including extension
	 * @return				index of the strategy in daytrender::strategies
	 */
	int init_strategy(const std::string& filename)
	{
		for (int i = 0; i < strategies.size(); i++)
		{
			if (strategies[i]->filename() == filename)
			{
				return i;
			}
		}

		Strategy* strat = new Strategy(dtdir + STRATEGY_DIR + filename);

		if (strat->is_bound())
		{
			strategies.push_back(strat);
			successf("Successfully initialized strategy: '%s'", filename);
			return strategies.size() - 1;
		}
		else
		{
			errorf("Strategy '%s' did not bind correctly", filename);
			delete strat;
			return -1;
		}
	}

	/**
	 * Frees all of the dynamically allocated memory used by daytrender.
	 * When deleting clients it closes all positions.
	 */
	void free()
	{
		mtx.lock();
		if (clients.empty())
		{
			warningf("No clients were initialized");
		}
		else
		{
			infof("Destructing clients...");
			for (int i = 0; i < clients.size(); i++)
			{
				infof("Closing all %s positions for %s", clients[i]->label(), clients[i]->filename());
				clients[i]->close_all_positions();
				delete clients[i];
			}
		}

		if (assets.empty())
		{
			warningf("No assets were initialized");
		}
		else
		{
			infof("Destructing assets...");
			for (int i = 0; i < assets.size(); i++)
			{
				delete assets[i];	
			}
		}

		if (strategies.empty())
		{
			warningf("No strategies were initialized");
		}
		else
		{
			infof("Destructing strategies...");
			for (int i = 0; i < strategies.size(); i++)
			{
				delete strategies[i];
			}
		}
		mtx.unlock();
	}

	void start()
	{
		mtx.lock();

		if (running)
		{
			warningf("DayTrender has already started!");
			mtx.unlock();
			return;
		}

		if (!shouldrun)
		{
			fatalf("Execution of DayTrender cannot continue!");
			mtx.unlock();
			return;
		}

		running = true;
		mtx.unlock();

		std::thread shell_thread(shell::get_input);
		std::thread server_thread(server::start);
		shell_thread.detach();
		infof("Starting DayTrender");

		while (running)
		{
			// check if every asset needs to update every 500ms, update each asset as needed 
			for (Asset* asset : assets)
			{
				if (asset->is_live())
				{
					asset->update();
				}
			}

			for (Client* client : clients)
			{
				client->update();
			}
			sys::thread_sleep(500);
		}

		server::stop();
		server_thread.join();
	}

	void stop()
	{
		mtx.lock();
		if (!running)
		{
			warningf("DayTrender has already stopped");
			return;
		}

		running = false;
		infof("Shutting down DayTrender...");
		mtx.unlock();
	}

	bool is_running()
	{
		return running;
	}

	std::vector<std::string> client_names()
	{
		std::vector<std::string> out(clients.size());

		for (int i = 0; i < clients.size(); i++)
		{
			out[i] = clients[i]->label();
		}

		return out;
	}

	std::vector<std::string> strategy_names()
	{
		std::vector<std::string> out(strategies.size());

		for (int i = 0; i < strategies.size(); i++)
		{
			out[i] = strategies[i]->filename(); 
		}

		return out;
	}

	std::vector<std::pair<std::string, int>> asset_names()
	{
		std::vector<std::pair<std::string, int>> out(assets.size());

		for (int i = 0; i < assets.size(); i++)
		{
			out[i] = { assets[i]->ticker(), assets[i]->type() };
		}

		return out;
	}

	const Asset* get_asset(int index) { return assets[index]; }
	const Strategy* get_strategy(int index) { return strategies[index]; }
	const Client* get_client(int type) { return clients[type]; }
}

int main(int argc, char *argv[])
{
	hirzel::fountain::init("./report.log", true, true, false, 1);
	daytrender::init(hirzel::str::get_folder(argv[0]));
	daytrender::start();
	daytrender::free();
	successf("DayTrender has stopped");
	return 0;
}