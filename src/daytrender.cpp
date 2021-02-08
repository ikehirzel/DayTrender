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
	std::vector<Algorithm*> algorithms;
	std::vector<Client*> clients;

	std::mutex mtx;

	bool init_client(const json& config);
	bool init_asset(const json& config, int type);
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
			init_client(clients_json[i]);
		}

		infof("Initialized clients:       %d", clients.size());
		infof("Initialized assets:        %d", assets.size());
		infof("Initialized algorithms:    %d", algorithms.size());

		json server_json = json::parse(server_str);

		if(!server::init(server_json, dtdir))
		{
			errorf("Failed to initialize server! aborting...");
			shouldrun = false;
		}

		mtx.unlock();
		shouldrun = false;

		// auto acct = clients[0]->get_account_info();
		// printfmt("Balance: %f\nBuying_power: %f\nEquity: %f\nLeverage: %d\n\n", acct.balance(), acct.buying_power(), acct.equity(), acct.leverage());

		// clients[0]->market_order("EUR_USD", -40000);

		// sys::thread_sleep(1000);
		// acct = clients[0]->get_account_info();
		// printfmt("Balance: %f\nBuying_power: %f\nEquity: %f\nLeverage: %d\n\n", acct.balance(), acct.buying_power(), acct.equity(), acct.leverage());

		// clients[0]->market_order("EUR_USD", 80000);

		// sys::thread_sleep(1000);
		// acct = clients[0]->get_account_info();
		// printfmt("Balance: %f\nBuying_power: %f\nEquity: %f\nLeverage: %d\n\n", acct.balance(), acct.buying_power(), acct.equity(), acct.leverage());

		// clients[0]->market_order("EUR_USD", -40000);

		// sys::thread_sleep(1000);
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

	bool init_client(const json& config)
	{
		std::string filename = config["filename"].get<std::string>();
		// checking if vars are defined
		if (!check_is_defined(filename, config, {
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
		if (!check_is_in_range(filename, config, 0, 1.0, {
			"max_loss",
			"risk",
		})) return false;

		// checking if vars are at least 0
		if (!check_is_in_range(filename, config, 0, 0, {
			"history_length",
			"leverage",
			"closeout_buffer"
		})) return false;

		const json& credentials = config["keys"];
		std::string label = config["label"].get<std::string>();
		double history_length = config["history_length"].get<double>();
		double max_loss = config["max_loss"].get<double>();
		int leverage = config["leverage"].get<int>();
		bool shorting_enabled = config["shorting_enabled"].get<bool>();
		double risk = config["risk"].get<double>();
		int closeout_buffer = config["closeout_buffer"].get<int>();
		std::vector<std::string> args(credentials.begin(), credentials.end());

		/*
			TODO:
			Add an "expected credential count" to clients so that this can be upgraded to an error
		*/



		Client* cli = new Client(label, dtdir + CLIENTS_DIR + filename, args, shorting_enabled,
			risk, max_loss, leverage, history_length, closeout_buffer);
	
		if (credentials.size() != cli->key_count())
		{
			errorf("%s: Expected %d keys but %s were supplied", filename, cli->key_count(), credentials.size());
			delete cli;
			return false;
		}

		if (!cli->bound())
		{
			errorf("%s: Client failed to bind", filename);
			delete cli;
			return false;
		}

		if (!cli->is_live())
		{
			errorf("%s: Client could not go live", label);
			delete cli;
			return false;
		}

		clients.push_back(cli);

		infof("%s: Initializing %s assets...", filename, label);
		// allocating all the assets for the client;
		const json& assets_json = config["assets"];
		for (int i = 0; i < assets_json.size(); i++)
		{
			init_asset(assets_json[i], clients.size() - 1);
		}
		successf("Successfully initialized %s client: '%s'", label, filename);
		return true;
	}

	bool init_asset(const json& config, int type)
	{
		std::string ticker = config["ticker"].get<std::string>();

		if (!check_is_defined(ticker, config, {
			"ticker",
			"algorithm",
			"interval",
			"ranges"
		})) return false;

		std::string algorithm_filename = config["algorithm"];
		int interval = config["interval"].get<int>();
		std::vector<int> ranges(config["ranges"].begin(), config["ranges"].end());

		if (clients[type]->to_interval(interval) == "")
		{
			errorf("%s: Interval was improperly defined", ticker);
			return false;
		}

		int strat_index = init_strategy(algorithm_filename);
		if (strat_index < 0)
		{
			errorf("%s: Asset cannot be initialized without strategy", ticker);
			return false;
		}

		if (ranges.size() != algorithms[strat_index]->indicator_count())
		{
			errorf("%s: Expected %d ranges but %d were supplied", ticker, algorithms[strat_index]->indicator_count(), ranges.size());
			return false;
		}

		assets.push_back(new Asset(type, clients[type], ticker, algorithms[strat_index], interval, ranges));	
		successf("Successfully initialized asset: '%s'", ticker);
		return true;
	}

	int init_strategy(const std::string& filename)
	{
		for (int i = 0; i < algorithms.size(); i++)
		{
			if (algorithms[i]->filename() == filename)
			{
				return i;
			}
		}

		Algorithm* algo = new Algorithm(dtdir + ALGORITHM_DIR + filename);

		if (algo->is_bound())
		{
			algorithms.push_back(algo);
			successf("Successfully initialized strategy: '%s'", filename);
			return algorithms.size() - 1;
		}
		else
		{
			errorf("Algorithm '%s' did not bind correctly", filename);
			delete algo;
			return -1;
		}
	}


	// destroys clients and assets
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
				warningf("Not actually closing positions");
				//infof("Closing all %s positions for %s", clients[i]->label(), clients[i]->filename());
				//clients[i]->close_all_positions();
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

		if (algorithms.empty())
		{
			warningf("No algorithms were initialized");
		}
		else
		{
			infof("Destructing algorithms...");
			for (int i = 0; i < algorithms.size(); i++)
			{
				delete algorithms[i];
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

	std::vector<std::string> algorithm_names()
	{
		std::vector<std::string> out(algorithms.size());

		for (int i = 0; i < algorithms.size(); i++)
		{
			out[i] = algorithms[i]->filename(); 
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
	const Algorithm* get_algorithm(int index) { return algorithms[index]; }
	Client* get_client(int type) { return clients[type]; }
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