#include "daytrender.h"

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

#include "data/candle.h"
#include "data/asset.h"
#include "data/interval.h"

#include "api/action.h"
#include "api/tradeclient.h"
#include "api/tradealgorithm.h"

#include "interface/shell.h"
#include "interface/server.h"

#include <filesystem>
#include <thread>
#include <mutex>

using namespace hirzel;

namespace daytrender
{
	bool running = false, shouldrun = true;
	std::string dtdir;

	// data containers
	std::vector<Asset*> assets;
	std::vector<TradeAlgorithm*> algorithms;
	std::vector<TradeClient*> clients;
	std::string compiler, buildflags;
	json config;

	std::mutex mtx;

	void update();
	bool initClients();
	bool initAssets();
	bool initBuilder();
	bool loadConfig();

	void init(const std::string& execpath)
	{
		mtx.lock();

		dtdir = std::filesystem::current_path().string() + "/" + execpath;

		//**************************************************

		infof("Initializing clients...");
		// loading credentials for apis

		std::string config_str = file::read_file_as_string(dtdir + "/config.json");

		if (config_str.empty())
		{
			fatalf("Failed to load config.json! aborting...");
			shouldrun = false;
			mtx.unlock();
			return;
		}
		else
		{
			successf("Successfully loaded config.json");
		}

		config = json::parse(config_str);

		std::cout << "config size: " << config.size() << std::endl;
		json& clients_json = config["clients"];

		std::cout << "clients json size: " << clients_json.size() << std::endl;

		for (int i = 0; i < clients_json.size(); i++)
		{
			json& client_json = clients_json[i];
			json& credentials = client_json["keys"];
			std::string label = client_json["label"];
			std::string filename = client_json["filename"];
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

			TradeClient* client = new TradeClient(label, filename, args);

			if (!client->all_bound())
			{
				delete client;
				errorf("%s client %s failed to bind!", label, filename);
				continue;
			}
			else
			{
				clients[i] = client;
			}

			// allocating all the assets for the client;

			json& assets_json = client_json["assets"];

			for (int j = 0; j < assets_json.size(); i++)
			{
				json& asset_json = assets_json[j];

				std::string ticker = asset_json["ticker"];
				std::string algorithm = asset_json["algorithm"];
				bool paper = asset_json.get<bool>();
				int interval = asset_json.get<int>();
				double risk = asset_json.get<double>();
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

				TradeAlgorithm* algo = nullptr;

				for (int k = 0; k < algorithms.size(); i++)
				{
					if (algorithms[k]->get_filename() == algorithm)
					{
						algo = algorithms[k];
						break;
					}
				}

				if (!algo)
				{
					algo = new TradeAlgorithm(dtdir + algorithm);
					algorithms.push_back(algo);
				}

				if (!algo->is_bound())
				{
					errorf("Algorithm '%s' did not bind correctly. '%s' cannot be initialized.", algorithm, ticker);
				}
				else
				{
					assets.push_back(new Asset(i, client, ticker, algo, interval, risk, ranges, paper));
				}
			}

		}

		// frees unused algorithms
		for (int i = algorithms.size() - 1; i >= 0; i--)
		{
			if (!algorithms[i]->is_bound())
			{
				delete algorithms[i];
				algorithms.erase(algorithms.begin() + i);
			}
		}

		//**************************************************
		// sets the callbacks for the server
		if(!server::init(config["Server"], dtdir))
		{
			errorf("Failed to initialize server! aborting...");
			shouldrun = false;
		}
		else
		{
			successf("Successfully initialized server");
		}

		mtx.unlock();
	}

	// destroys clients and assets
	void free()
	{
		mtx.lock();

		for (int i = 0; i < clients.size(); i++)
		{
			delete clients[i];
		}

		int count = 0;
		for (int i = 0; i < assets.size(); i++)
		{
			delete assets[i];
			count++;
			
		}
		if (!count) warningf("No assets were initialized!");

		count = 0;
		for (TradeAlgorithm * algo : algorithms)
		{
			if (algo)
			{
				delete algo;
				count++;
			}
		}
		if(!count) warningf("No algorithms were initialized!");

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

		std::thread shellInputThread(shell::get_input);
		std::thread serverThread(server::start);

		long long time, last, elapsed, timeout;

		time = sys::get_millis();
		last = time - time % 60000;

		timeout = (61000 - (sys::get_millis() % 60000)) % 60000;
		std::string msg = "Timeout: " + std::to_string((double)timeout / 1000.0) + " seconds";

		if (timeout < 10000)
		{
			msg += ": Resting...";
			infof(msg);
		}
		else
		{
			infof(msg);
			update();
		}

		while (running)
		{
			time = sys::get_millis();
			elapsed = time - last;

			if (time % 60000 >= 1000 && time % 60000 < 2000 && elapsed >= 2000)
			{
				last = time;
				update();
			}

			sys::thread_sleep(200);
		}

		server::stop();
		shellInputThread.join();
		serverThread.join();
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

	void update()
	{
		infof("Updating assets...");
		unsigned int count = 0;

		for (Asset *a : assets)
		{
			if(a->isLive())
			{
				count++;
				a->update();
			}
		}

		if(!count)
		{
			warningf("No assets were live for updating");
		}
		else
		{
			successf("Updated %d assets", count);
		}
	}

	std::vector<PaperAccount> backtest(int algo_index, int asset_index, const std::vector<int>& test_ranges)
	{
		std::vector<PaperAccount> out;
		std::string ticker;
		TradeClient* client = nullptr;
		TradeAlgorithm* algo = nullptr;

		std::vector<candleset> candles_vec;
		std::vector<int> intervals;
		double paper_initial, paper_minimum, paper_fee, risk;
		int asset_type;
		
		asset_type = assets[asset_index]->getType();
		risk = assets[asset_index]->getRisk();
		client = clients[asset_type];
		ticker = assets[asset_index]->getTicker();
		algo = algorithms[algo_index];

		paper_initial = PAPER_ACCOUNT_INITIAL;
		paper_fee = client->paper_fee();
		paper_minimum = client->paper_minimum();
		intervals = client->backtest_intervals();

		out.resize(intervals.size());

		candles_vec.resize(intervals.size());
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
				algorithm_data data;  
				PaperAccount acc(PAPER_ACCOUNT_INITIAL, paper_fee, paper_minimum, intervals[i], ranges);

				data.ranges = ranges.data();

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
				for (int k = 0; k < candles_vec[i].size - candle_count; k++)
				{
					candleset candles = candles_vec[i].get_slice(k, candle_count);
					acc.setPrice(candles.back().close);
					algorithm_data data = algo->process(candles, ranges);

					if (data.err)
					{
						errorf("Backtest: %s", data.err);
					}
					else
					{
						action::paper_actions[data.action](acc, risk);
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

		for (int i = 0; i < candles_vec.size(); i++)
		{
			candles_vec[i].clear();
		}

		return out;
	}

	bool isRunning()
	{
		return running;
	}

	std::vector<std::string> getClientInfo()
	{
		std::vector<std::string> out;

		for (int i = 0; i < clients.size(); i++)
		{
			out[i] = clients[i]->get_label();
		}

		return out;
	}

	std::vector<std::string> getAlgoInfo()
	{
		std::vector<std::string> out(algorithms.size());

		for (int i = 0; i < algorithms.size(); i++)
		{
			out[i] = algorithms[i]->get_filename(); 
		}

		return out;
	}

	std::vector<std::pair<std::string, int>> getAssetInfo()
	{
		std::vector<std::pair<std::string, int>> out(assets.size());

		for (int i = 0; i < assets.size(); i++)
		{
			out[i] = { assets[i]->getTicker(), assets[i]->getType() };
		}

		return out;
	}

	const Asset* getAsset(int index)
	{
		return assets[index];
	}

	const TradeClient* getClient(int type)
	{
		return clients[type];
	}
}
