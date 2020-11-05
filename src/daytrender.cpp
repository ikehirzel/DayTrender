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
//#define CPPHTTPLIB_OPENSSL_SUPPORT
//#include <httplib.h>

#include <filesystem>
#include <functional>
using namespace hirzel;
//using namespace httplib;

#define CONFIG_FOLDER			"/config/"
#define RESOURCES_FOLDER		"/resources/"
#define SCRIPT_FOLDER			"/algorithms/"
#define ALGORITHM_BIN_FOLDER	RESOURCES_FOLDER"bin/"
#define HTML_FOLDER				RESOURCES_FOLDER"html/"

namespace daytrender
{
	DayTrender::DayTrender(const std::string& execpath)
	{
		dtdir = std::filesystem::current_path().string() + "/" + execpath;
		// NOTE: functions must be called in this order

		// loads the credentials of the clients and creates them
		initClients();
		// loads the asset data and creates assets and their respective algorithms
		initAssets();
		// sets the callbacks for the server
		initServer();
		shouldrun = false;
		backtest("simplema", "forex", "EUR_USD");

	}

	// destroys clients and assets
	DayTrender::~DayTrender()
	{
		(forex) ? delete forex : warningf("Forex was never initialized!");
		(stocks) ? delete stocks : warningf("Forex was never initialized!");
		//(server) ? delete server : warningf("Server was never initialized!");

		unsigned int count = 0;
		for (unsigned int i = 0; i < assets.size(); i++)
		{
			count = 0;
			for (Asset *asset : assets[i])
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
	}

	// loads clients credentials and creates clients
	void DayTrender::initClients()
	{
		infof("Initializing clients...");
		// loading credentials for apis
		std::string credentialStr = read_string(dtdir + CONFIG_FOLDER "keys.json");
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

		stocks = new AlpacaClient({alpacaCredentials["key"],
								   alpacaCredentials["secret"]});

		// Setting oanda credentials
		json oandaCredentials = credentials["Oanda"];
		
		forex = new OandaClient({oandaCredentials["username"].get<std::string>(),
								 oandaCredentials["accountid"].get<std::string>(),
								 oandaCredentials["token"].get<std::string>()});
	}

	bool DayTrender::buildAlgorithm(const std::string& filename, bool print)
	{
		infof("Compiling %s...", filename);
		std::string cmd = dtdir + "/dtbuild " + dtdir + SCRIPT_FOLDER + filename + ".alg -o ";
		if (print) cmd += "-p ";
		cmd += dtdir + ALGORITHM_BIN_FOLDER ".";
		return !std::system(cmd.c_str());
	}

	bool DayTrender::loadAlgorithm(const std::string& filename)
	{
		infof("Loading %s...", filename);
		// initializing algorithm
		// if an algo failed to load and another one uses it,it will try to load it again
		// maybe this needs to be optimized with some kind of flag
		if(!algorithms[filename])
		{
			//compile algorithm
			bool success = false;
			#ifdef JIT_COMPILE_ALGORITHMS
			success = buildAlgorithm(filename, false);
			#else
			std::string binpath = dtdir + ALGORITHM_BIN_FOLDER + filename + ALGORITHM_EXTENSION;
			hirzel::printf("binpath: %s\n", binpath);
			if(!hirzel::file_exists(binpath))
			{
				success = buildAlgorithm(filename);
			}
			#endif
			// load algorithm into memory
			algorithms[filename] = new TradeAlgorithm(dtdir + ALGORITHM_BIN_FOLDER + filename + ALGORITHM_EXTENSION);
			if(!success || !algorithms[filename]->isBound())
			{
				return false;
			}
		}
		return true;
	}

	// loads asset info, creates assets, and loads algorithm names
	void DayTrender::initAssets()
	{
		infof("Initializing assets...");
		// Loading asset info
		std::string assetStr = read_string(dtdir + CONFIG_FOLDER "assets.json");
		if (assetStr.empty())
		{
			fatalf("Failed to load ." CONFIG_FOLDER "assets.json!");
			shouldrun = false;
			return;
		}
		successf("Successfully loaded ." CONFIG_FOLDER "assets.json");

		json assetInfo = json::parse(assetStr);

		assets.resize(ASSET_TYPE_COUNT);

		std::vector<TradeClient*> clients;
		clients.resize(ASSET_TYPE_COUNT);
		clients[FOREX_INDEX] = forex;

		// creating assets
		for (unsigned int i = 0; i < ASSET_TYPE_COUNT; i++)
		{
			for (json asset : assetInfo[asset_labels[i]])
			{
				unsigned int interval, window;
				std::string ticker, algo_filename;

				ticker = asset["ticker"].get<std::string>();
				algo_filename = asset["algorithm"].get<std::string>();
				interval = asset["interval"].get<int>();
				window = asset["window"].get<int>();

				if(!loadAlgorithm(algo_filename))
				{
					assets[i].push_back(new Asset(i, clients[i], ticker, nullptr, interval, window));
				}
				else
				{
					assets[i].push_back(new Asset(i, clients[i], ticker, algorithms[algo_filename], interval, window));
				}
			}
		}


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

	void DayTrender::initServer()
	{
		infof("Loading server data...");
		std::string data = read_string(dtdir + CONFIG_FOLDER "serverinfo.json");
		if(data.empty())
		{
			fatalf("Failed to read ." CONFIG_FOLDER "serverinfo.json!");
			shouldrun = false;
			return;
		}
		successf("Successfully loaded ." CONFIG_FOLDER "serverinfo.json");
		json serverInfo = json::parse(data);

		//this->ip = serverInfo["ip"].get<std::string>();
		//this->port = serverInfo["port"].get<int>();
		//this->username = serverInfo["username"].get<std::string>();
		//this->password = serverInfo["password"].get<std::string>();
		//this->server = new httplib::Server();

		infof("Initializing server...");
	}

	void DayTrender::start()
	{
		if (running)
		{
			warningf("DayTrender has already started!");
			return;
		}

		if (!shouldrun)
		{
			fatalf("Execution of DayTrender cannot continue!");
			return;
		}

		infof("Starting DayTrender...");
		running = true;

		//serverThread = std::thread(&httplib::Server::listen, server, ip.c_str(), port, 0);
		conioThread = std::thread(&DayTrender::scanInput, this);

		long long time, last, elapsed, timeout;

		time = getMillis();
		last = time - time % 60000;

		timeout = (61000 - (getMillis() % 60000)) % 60000;
		std::string msg = "Timeout: " + std::to_string((double)timeout / 1000.0) + " seconds";

		if (timeout < 10000)
		{
			msg += ": Resting...";
			infof(msg.c_str());
		}
		else
		{
			infof(msg.c_str());
			update();
		}

		while (running)
		{
			time = getMillis();
			elapsed = time - last;

			if (time % 60000 >= 1000 && time % 60000 < 2000 && elapsed >= 2000)
			{
				last = time;
				update();
			}

			thread_sleep(200);
		}
		// join threads
		//serverThread.join();
		conioThread.join();
	}

	void DayTrender::update()
	{
		infof("Updating assets...");
		unsigned int count = 0;
		for (std::vector<Asset *> vec : assets)
		{
			for (Asset *a : vec)
			{
				if(a->isLive())
				{
					count++;
					a->update();
				}
			}
		}
		if(!count)
		{
			warningf("No assets were live for updating");
		}
	}

	void DayTrender::stop()
	{
		if (!running)
		{
			warningf("DayTrender has already stopped");
			return;
		}

		running = false;
		//server->stop();
		infof("Shutting down...");
	}

	void DayTrender::backtest(const std::string& algoname, const std::string& clientname, const std::string& ticker)
	{
		TradeClient* client = nullptr;
		TradeAlgorithm* algo = nullptr;
		std::vector<candleset> candles_vec;
		unsigned int asset_index = 0;
		double paper_initial, paper_minimum, paper_fee;

		// getting algorithm pointer
		algo = algorithms[algoname];

		if(!algo)
		{
			errorf("Invalid algorithm name!");
			return;
		}

		// setting paper account information and getting client
		paper_initial = PAPER_ACCOUNT_INITIAL;
		if(clientname == "forex")
		{
			client = forex;
			paper_minimum = FOREX_MINIMUM;
			paper_fee = FOREX_FEE;
		}
		else if (clientname == "stocks")
		{
			client = stocks;
			paper_minimum = STOCK_MINIMUM;
			paper_fee = STOCK_FEE;
		}

		if(!client)
		{
			errorf("Invalid asset type!");
			return;
		}

		candles_vec.resize(3);

		candles_vec[0] = client->getCandles(ticker, backtest_intervals[asset_index][0]);
		candles_vec[1] = client->getCandles(ticker, backtest_intervals[asset_index][1]);
		candles_vec[2] = client->getCandles(ticker, backtest_intervals[asset_index][2]);
		

		for(const candleset& c : candles_vec)
		{
			if (c.empty())
			{
				errorf("Cannot continue backtest as not all candles were received!");
				return;
			}
		}

		PaperAccount best[3];
		for (unsigned int c = 0; c < candles_vec.size(); c++)
		{
			for (unsigned int window = MIN_ALGORITHM_WINDOW; window <= MAX_ALGORITHM_WINDOW; window += 5)
			{
				candleset candles;
				candles.resize(window);
				PaperAccount acc(paper_initial, paper_fee, paper_minimum, backtest_intervals[asset_index][c], window);

				for (unsigned int i = 0; i < candles_vec[c].size() - window; i++)
				{
					//setting value of candles to be passed to algorithm
					unsigned int index = 0;
					for (unsigned int j = i; j < i + window; j++)
					{
						candles[index] = candles_vec[c][j];
						index++;
					}

					acc.setPrice(candles.back().close);

					algorithm_data data = algo->process(candles);

					paper_actions[data.action](acc, 0.9);
				}

				// check if account is better than best
				if (acc.avgHourNetReturn() > best[c].avgHourNetReturn())
				{
					best[c] = acc;
				}
			}
			std::cout << "Best @ " << backtest_intervals[asset_index][c] << " " << best[c] << std::endl;
		}
	}

	void DayTrender::scanInput()
	{
		std::string input;

		while (running)
		{
			std::getline(std::cin, input);
			infof("dtshell: $ %s", input);
			std::vector<std::string> tokens = hirzel::tokenize(input, " \t");
			/************************************************************
			 * Exit
			 ***********************************************************/
			if(tokens[0] == "exit")
			{
				stop();
			}
			/************************************************************
			 *	Backtest
			 ************************************************************/
			else if (tokens[0] == "backtest")
			{
				std::string ticker;
				PaperAccount result, result1;
				bool found = false;
				// backtesting a specific asset
				if(tokens.size() == 2)
				{
					ticker = tokens[1];

						// searching through assets to find asset with that ticker
					for (std::vector<Asset*> list : assets)
					{
						for (Asset* a : list)
						{
							if(a->getTicker() == ticker)
							{
								//result = a->backtest();
								//result1 = a->backtest();
								found = false;
								break;
							}
						}
					}
					if (found)
					{
						std::cout << ticker << ": " << result << std::endl;
						std::cout << ticker << ": " << result1 << std::endl;
					}
					else
					{
						errorf("No asset '%s' was found!", ticker);
					}
				}
				// backtesting backtest simplesr forex
				else if (tokens.size() == 4)
				{
					backtest(tokens[1], tokens[2], tokens[3]);
				}
				else
				{
					errorf("Incorrect usage of command: either use backtest <ticker-name> or backtest <algorithm> <asset-type> <ticker>!");
				}

				
			}
			/************************************************************
			 *	Build
			 ************************************************************/
			else if (tokens[0] == "build")
			{
				bool build = true;
				std::string filename;
				bool print = false;
				for(unsigned int i = 1; i < tokens.size(); i++)
				{
					infof("Tok[%d]: %s", i, tokens[i]);
					if(tokens[i] == "-p")
					{

						print = true;
					}
					else
					{
						if (filename.empty())
						{
							filename = tokens[i];
						}
						else
						{
							errorf("Only one input file should be specified! Aborting...");
							build = false;
						}
					}
				}
				if (build)
				{
					build = buildAlgorithm(filename, print);
					if (build)
					{
						successf("Successfully compiled %s", tokens[1]);
					}
					else
					{
						errorf("Failed to compile %s", tokens[1]);
					}
				}
			}
			/************************************************************
			 *	Default
			 ************************************************************/
			else
			{
				warningf("dtshell: Command not found");
			}
		}
	} // scanInput

} // namespace daytrender
