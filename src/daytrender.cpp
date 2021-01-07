#include "daytrender.h"

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

#include "data/candle.h"
#include "data/action.h"
#include "data/asset.h"
#include "data/tradealgorithm.h"
#include "data/interval.h"

#include "api/alpacaclient.h"
#include "api/oandaclient.h"

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
	std::unordered_map<std::string, TradeAlgorithm*> algorithms;
	std::vector<TradeClient*> clients;

	std::mutex mtx;

	void update();
	void initClients();
	void initAssets();
	

	void init(const std::string& execpath)
	{
		mtx.lock();
		dtdir = std::filesystem::current_path().string() + "/" + execpath;
		
		// NOTE: functions must be called in this order

		// loads the credentials of the clients and creates them
		initClients();
		// loads the asset data and creates assets and their respective algorithms
		initAssets();
		// sets the callbacks for the server


		if(!server::init(dtdir))
		{
			errorf("Failed to initialize server!");
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
		for (unsigned i = 0; i < clients.size(); i++)
		{
			if (!clients[i])
			{
				warningf("%s client was never initialized", asset_labels[i]);
			}
			else
			{
				delete clients[i];
			}
		}

		unsigned int count = 0;
		for (unsigned i = 0; i < assets.size(); i++)
		{
			count = 0;
			for (Asset *asset : assets)
			{
				count++;
				delete asset;
			}
			if (!count) warningf("No %s assets were initialized!", asset_labels[i]);
		}

		count = 0;
		for (std::pair<std::string, TradeAlgorithm *> p : algorithms)
		{
			if(p.second)
			{
				delete p.second;
				count++;
			}
		}
		if(!count) warningf("No algorithms were initialized!");
		infof("End of destructor");
		mtx.unlock();
	}

	// loads clients credentials and creates clients
	void initClients()
	{
		infof("Initializing clients...");
		// loading credentials for apis
		clients.resize(ASSET_TYPE_COUNT);

		std::string credentialStr = file::read_file_as_string(dtdir + CONFIG_FOLDER "keys.json");
		if (credentialStr.empty())
		{
			fatalf("Failed to load '." CONFIG_FOLDER "'!");
			shouldrun = false;
			return;
		}
		successf("Successfully loaded ." CONFIG_FOLDER "keys.json");
		json credentials = json::parse(credentialStr);

		// Setting Alpaca credentials
		json alpacaCredentials = credentials["Alpaca"];

		clients[STOCK_INDEX] = new AlpacaClient({alpacaCredentials["key"],
								   alpacaCredentials["secret"]});

		// Setting oanda credentials
		json oandaCredentials = credentials["Oanda"];
		
		clients[FOREX_INDEX] = new OandaClient({oandaCredentials["username"].get<std::string>(),
								 oandaCredentials["accountid"].get<std::string>(),
								 oandaCredentials["token"].get<std::string>()});
	}

	bool buildAlgorithm(const std::string& filename, bool print)
	{
		infof("Compiling %s...", filename);
		std::string cmd = dtdir + "/dtbuild " + dtdir + SCRIPT_FOLDER + filename + ".alg -o ";
		if (print) cmd += "-p ";
		cmd += dtdir + ALGORITHM_BIN_FOLDER ".";
		return !std::system(cmd.c_str());
	}

	bool loadAlgorithm(const std::string& filename)
	{
		infof("Loading %s...", filename);
		// initializing algorithm
		// if an algo failed to load and another one uses it,it will try to load it again
		// maybe this needs to be optimized with some kind of flag
		if(!algorithms[filename])
		{
			algorithms[filename] = new TradeAlgorithm(dtdir + ALGORITHM_BIN_FOLDER + filename + ALGORITHM_EXTENSION);
			if(!algorithms[filename]->isBound())
			{
				return false;
			}
		}
		return true;
	}

	// loads asset info, creates assets, and loads algorithm names
	void initAssets()
	{
		infof("Initializing assets...");
		// Loading asset info

		std::string assetStr = file::read_file_as_string(dtdir + CONFIG_FOLDER "assets.json");
		if (assetStr.empty())
		{
			errorf("Failed to load ." CONFIG_FOLDER "assets.json! Creating blank file...");
			assetStr = "{\n\t\"Forex\": [],\n\t\"Stocks\": []\n}\n";
			file::write_file(dtdir + CONFIG_FOLDER "assets.json", assetStr);
			return;
		}
		successf("Successfully loaded ." CONFIG_FOLDER "assets.json");

		json assetInfo = json::parse(assetStr);

		if (!assets.empty()) assets.clear();

		// creating assets
		for (int i = 0; i < ASSET_TYPE_COUNT; i++)
		{
			for (json asset : assetInfo[asset_labels[i]])
			{
				int interval, window;
				std::string ticker, algo_filename;
				bool paper;

				ticker = asset["ticker"].get<std::string>();
				algo_filename = asset["algorithm"].get<std::string>();
				interval = asset["interval"].get<int>();
				paper = asset["paper"].get<bool>();
				json& jranges = asset["ranges"];

				std::vector<int> ranges(jranges.size());
				for (int j = 0; j < jranges.size(); j++)
				{
					ranges[j] = jranges[j];
				}

				// if algo failed to load
				if(!loadAlgorithm(algo_filename))
				{
					assets.push_back(new Asset(i, clients[i], ticker, nullptr, interval, ranges, paper));
				}
				else
				{
					assets.push_back(new Asset(i, clients[i], ticker, algorithms[algo_filename], interval, ranges, paper));
				}
			}
		}

		std::cout << "Assets: " << assets.size() << std::endl;

		// if the algorithm plugin failed to load, it will clear the memory to avoid assets attempting to use it
		for (std::pair<std::string, TradeAlgorithm*> p : algorithms)
		{
			if (!p.second->isBound())
			{
				warningf("Disposing of uninitialized algorithm '%s'", p.first);
				delete p.second;
				algorithms[p.first] = nullptr;
			}
		}
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

		std::thread shellInputThread(shell_getInput);
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
	}

	std::vector<PaperAccount> backtest(const std::string& algoname, const std::string& clientname, const std::string& ticker)
	{
		std::vector<PaperAccount> out;
		TradeClient* client = nullptr;
		TradeAlgorithm* algo = nullptr;
		std::vector<candleset> candles_vec;
		int asset_index = 0;
		double paper_initial, paper_minimum, paper_fee;

		// getting algorithm pointer
		algo = algorithms[algoname];

		if(!algo)
		{
			errorf("Invalid algorithm name!");
			return {};
		}

		// setting paper account information and getting client
		paper_initial = PAPER_ACCOUNT_INITIAL;
		if(clientname == "forex")
		{
			client = clients[FOREX_INDEX];
			paper_minimum = FOREX_MINIMUM;
			paper_fee = FOREX_FEE;
			asset_index = FOREX_INDEX;
		}
		else if (clientname == "stocks")
		{
			client = clients[STOCK_INDEX];
			paper_minimum = STOCK_MINIMUM;
			paper_fee = STOCK_FEE;
			asset_index = STOCK_INDEX;
		}

		if(!client)
		{
			errorf("Invalid asset type!");
			return {};
		}

		out.resize(3);

		candles_vec.resize(3);
		for (int i = 0; i < 3; i++)
		{
			candles_vec[i] = client->getCandles(ticker, backtest_intervals[asset_index][i]);

			if (candles_vec[i].empty())
			{
				errorf("Cannot continue backtest as not all candles were received!");
				return {};
			}
		}

		infof("Backtesting algorithm...");

		//***********************************
		// new and improved forloop structure
		//***********************************

		// calculating lops
		std::vector<int> ranges(algo->arg_count(), MIN_ALGORITHM_WINDOW);
		#define SKIP_AMT 5
		long long permutations = 1;
		int possible_vals = ((MAX_ALGORITHM_WINDOW + 1) - MIN_ALGORITHM_WINDOW) / SKIP_AMT;
		for (int i = 0; i < algo->arg_count(); i++) permutations *= possible_vals;
		
		// for every candleset in the bunch
		for (int i = 0; i < 3; i++)
		{
			PaperAccount best;
			double bahnr = 0.0;
			// re-init with minimum range for each arg
			ranges = std::vector<int>(algo->arg_count(), MIN_ALGORITHM_WINDOW);

			// for every possible permutation of ranges for indicators/algorithm
			for (int j = 0; j < permutations; j++)
			{
				// storing activity and performance data
				algorithm_data data;
				PaperAccount acc(PAPER_ACCOUNT_INITIAL, paper_fee, paper_minimum, backtest_intervals[asset_index][i], ranges);

				data.ranges = ranges;

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
					data.candles = candles_vec[i].get_slice(k, candle_count);
					acc.setPrice(data.candles.back().close);

					if (!algo->process(data))
					{
						std::cout << "Error while backtesting: " << data.err << std::endl;
					}
					else
					{
						paper_actions[data.action](acc, 0.9);
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

			std::cout << "Done\n";
			out[i] = best;
			// this makes it so it only happens once (for testing purposes)
			break;
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

	std::vector<std::pair<std::string, std::string>> getAlgoInfo()
	{
		std::vector<std::pair<std::string, std::string>> out;
		out.resize(algorithms.size());

		unsigned index = 0;

		for (std::pair<std::string, TradeAlgorithm*> p : algorithms)
		{
			out[index].first = p.first; 
			out[index].second = p.second->getName();
			std::cout << "Filename: " << out[index].second << std::endl;
			index++;
		}

		return out;
	}

	std::vector<std::pair<std::string, unsigned>> getAssetInfo()
	{
		std::vector<std::pair<std::string, unsigned>> out;
		out.resize(assets.size());

		for (unsigned i = 0; i < assets.size(); i++)
		{
			out[i] = { assets[i]->getTicker(), assets[i]->getType() };
		}

		return out;
	}

	const Asset* getAsset(unsigned index)
	{
		return assets[index];
	}
}
