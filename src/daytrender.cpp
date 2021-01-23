#include "daytrender.h"

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

#include "data/candle.h"
#include "data/asset.h"
#include "data/interval.h"

#include "api/action.h"
#include "api/client.h"
#include "api/algorithm.h"

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

	void init(const std::string& execpath)
	{
		mtx.lock();

		dtdir = std::filesystem::current_path().string() + "/" + execpath;

		//**************************************************

		infof("Initializing clients...");
		// loading credentials for apis

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

		json clients_json = json::parse(clients_str);
		for (int i = 0; i < clients_json.size(); i++)
		{
			json& client_json = clients_json[i];
			json& credentials = client_json["keys"];
			std::string label = client_json["label"];
			std::string filename = client_json["filename"];
			double risk = client_json["risk"].get<double>();
			std::vector<std::string> args(credentials.begin(), credentials.end());
			
			if (label.empty())
			{
				errorf("No label given for client[%d]!");
				continue;
			}
			
			if (filename.empty())
			{
				errorf("No filename given for %s client in config.json!", label);
				continue;
			}

			if (args.empty())
			{
				warningf("No credentials were passed to %s client in config.json", label);
			}

			Client* cli = new Client(label, dtdir + CLIENTS_DIR + filename, args, risk);

			if (cli->all_bound())
			{
				clients.push_back(cli);
			}
			else
			{
				errorf("%s client %s failed to bind!", label, filename);
				delete cli;
				continue;
			}

			// allocating all the assets for the client;
			json& assets_json = client_json["assets"];
			for (int j = 0; j < assets_json.size(); j++)
			{
				json& asset_json = assets_json[j];

				std::string ticker = asset_json["ticker"];
				std::string algorithm = asset_json["algorithm"];
				
				bool paper = asset_json["paper"].get<bool>();
				int interval = asset_json["interval"].get<int>();
				double risk = asset_json["risk"].get<double>();
				std::vector<int> ranges(asset_json["ranges"].begin(), asset_json["ranges"].end());

				if (ticker.empty())
				{
					errorf("No ticker defined for asset[%d] of %s client", j, label);
					continue;
				}

				if (algorithm.empty())
				{
					errorf("No algorithm defined for %s of %s client", ticker, label);
					continue;
				}

				if (interval == 0)
				{
					errorf("No interval defined for %s of %s client", ticker, label);
					continue;
				}
				else
				{
					std::cout << "init: interval: " << interval << std::endl;
				}

				if (risk == 0.0)
				{
					errorf("No risk defined for %s of %s client", ticker, label);
					continue;
				}

				if (ranges.empty())
				{
					errorf("No ranges defined for %s of %s client", ticker, label);
					continue;
				}

				int algoi = -1;
				for (int k = 0; k < algorithms.size(); i++)
				{
					if (algorithms[k]->get_filename() == algorithm)
					{
						algoi = k;
						break;
					}
				}

				if (algoi < 0)
				{
					algoi = algorithms.size();
					Algorithm* algo = new Algorithm(dtdir + ALGORITHM_DIR + algorithm);

					if (algo->is_bound())
					{
						algorithms.push_back(algo);
					}
					else
					{
						errorf("Algorithm '%s' did not bind correctly. '%s' cannot be initialized.", algorithm, ticker);
						delete algo;
						algorithms.pop_back();
						continue;
					}
				}

				assets.push_back(new Asset(i, clients[i], ticker, algorithms[algoi], interval, risk, ranges, paper));
				// incrementing the amount of assets for a client
				clients[i]->set_asset_count(clients[i]->get_asset_count() + 1);
				successf("Successfully initialized asset: '%s'", ticker);
			}
		}

		json server_json = json::parse(server_str);

		if(server::init(server_json, dtdir))
		{
			successf("Successfully initialized server");
		}
		else
		{
			errorf("Failed to initialize server! aborting...");
			shouldrun = false;
		}

		infof("Initialized clients:       %d", clients.size());
		infof("Initialized assets:        %d", assets.size());
		infof("Initialized algorithms:    %d", algorithms.size());

		mtx.unlock();
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
			for (int i = 0; i < clients.size(); i++)
			{
				delete clients[i];
			}
		}

		if (assets.empty())
		{
			warningf("No assets were initialized");
		}
		else
		{
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

		infof("Starting DayTrender...");
		running = true;
		mtx.unlock();

		//std::thread shellInputThread(shell::get_input);
		//std::thread serverThread(server::start);

		while (running)
		{
			// check if every asset needs to update every 500ms, update each asset as needed 
			for (Asset* asset : assets)
			{
				if (asset->should_update()) asset->update();
			}

			sys::thread_sleep(500);
		}

		//server::stop();
		//shellInputThread.join();
		//serverThread.join();
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
		infof("Shutting down...");
		mtx.unlock();
	}

	std::vector<PaperAccount> backtest(int algo_index, int asset_index, const std::vector<int>& test_ranges)
	{
		int asset_type = assets[asset_index]->type();
		double risk = assets[asset_index]->risk();
		const Client* client = clients[asset_type];
		std::string ticker = assets[asset_index]->ticker();
		const Algorithm* algo = algorithms[algo_index];

		double paper_initial = PAPER_ACCOUNT_INITIAL;
		double paper_fee = client->paper_fee();
		double paper_minimum = client->paper_minimum();
		std::vector<int> intervals = client->backtest_intervals();

		std::vector<PaperAccount> out(intervals.size());

		std::vector<CandleSet> candles_vec(intervals.size());
		for (int i = 0; i < candles_vec.size(); i++)
		{
			candles_vec[i] = client->get_candles(ticker, intervals[i]);

			if (candles_vec[i].empty())
			{
				errorf("Cannot continue backtest as not all candles were received!");
				return {};
			}
		}

		infof("Backtesting algorithm...");

		// calculating lops
		std::vector<int> ranges, start_ranges;
		#define SKIP_AMT 5
		long long permutations = 1;
		int possible_vals = ((MAX_ALGORITHM_WINDOW + 1) - MIN_ALGORITHM_WINDOW) / SKIP_AMT;
		// if no ranges are passed in
		if (test_ranges.empty())
		{
			// setting default ranges
			start_ranges.resize(algo->get_ranges_count(), MIN_ALGORITHM_WINDOW);

			// calculate the amount of permutations of ranges
			for (int i = 0; i < algo->get_ranges_count(); i++) permutations *= possible_vals;
		}
		else
		{
			// setting default ranges to the given ones
			start_ranges = test_ranges;

			// verify ranges size
			if (start_ranges.size() !=  algo->get_ranges_count())
			{
				errorf("Passed in %d ranges to backtest but %d were expected! resizing ranges...", start_ranges.size(), algo->get_ranges_count());
				start_ranges.resize(algo->get_ranges_count());
			}

			// verify minimums and maximums
			for (int i = 0; i < ranges.size(); i++)
			{
				if (start_ranges[i] < MIN_ALGORITHM_WINDOW)
				{
					start_ranges[i] = MIN_ALGORITHM_WINDOW;
					warningf("test_ranges[%d] was less than the minimum (%d). Readjusting...", start_ranges[i], MIN_ALGORITHM_WINDOW);
				}
				else if (start_ranges[i] > MAX_ALGORITHM_WINDOW)
				{
					start_ranges[i] = MAX_ALGORITHM_WINDOW;
					warningf("test_ranges[%d] was greater than the maximum (%d). Readjusting...", start_ranges[i], MIN_ALGORITHM_WINDOW);
				}
			}
		}

		// for every intervals
		for (int i = 0; i < intervals.size(); i++)
		{
			PaperAccount best;
			double bahnr = 0.0;
			// re-init with minimum range for each arg
			ranges = start_ranges;

			// for every possible permutation of ranges for indicators/algorithm
			for (int j = 0; j < permutations; j++)
			{
				// storing activity and performance data
				PaperAccount acc(PAPER_ACCOUNT_INITIAL, paper_fee, paper_minimum, intervals[i], ranges);

				// calculating size of candles
				int candle_count = 0;
				int longest_indi = 0;
				if (ranges.size() > 1)
				{
					for (int v = 1; v < ranges.size(); v++)
					{
						if (ranges[v] > longest_indi) longest_indi = ranges[v];
					}
				}
				candle_count = ranges[0] + longest_indi;
				//std::cout << "candle count: " << candle_count << std::endl;

				// walk through every step of candles_vec[i]
				for (int k = 0; k < candles_vec[i].size() - candle_count; k++)
				{
					CandleSet candles = candles_vec[i].slice(k, candle_count);
					acc.setPrice(candles.back().close);
					AlgorithmData data = algo->process(candles, ranges);

					if (data.error())
					{
						errorf("Backtest: %s", data.error());
					}
					else
					{
						action::paper_actions[data.action()](acc, risk);
					}
				}

				// comparing account to previous, potentially storing account

				double ahnr = acc.avgHourNetReturn();
				
				if (ahnr > bahnr)
				{
					bahnr = ahnr;
					best = acc;
				}

				// incrementing the permutation

				int pos = 0;
				ranges[pos] += SKIP_AMT;
				while (ranges[pos] > MAX_ALGORITHM_WINDOW)
				{
					ranges[pos] = MIN_ALGORITHM_WINDOW;
					pos++;
					ranges[pos] += SKIP_AMT;
				}
			}

			std::cout << "Completed backtest " << i << '\n';
			out[i] = best;
		}

		return out;
	}

	bool is_running()
	{
		return running;
	}

	std::vector<std::string> get_client_info()
	{
		std::vector<std::string> out;

		for (int i = 0; i < clients.size(); i++)
		{
			out[i] = clients[i]->get_label();
		}

		return out;
	}

	std::vector<std::string> get_algo_info()
	{
		std::vector<std::string> out(algorithms.size());

		for (int i = 0; i < algorithms.size(); i++)
		{
			out[i] = algorithms[i]->get_filename(); 
		}

		return out;
	}

	std::vector<std::pair<std::string, int>> get_asset_info()
	{
		std::vector<std::pair<std::string, int>> out(assets.size());

		for (int i = 0; i < assets.size(); i++)
		{
			out[i] = { assets[i]->ticker(), assets[i]->type() };
		}

		return out;
	}

	const Asset* get_asset(int index)
	{
		return assets[index];
	}

	const Client* get_client(int type)
	{
		return clients[type];
	}
}
