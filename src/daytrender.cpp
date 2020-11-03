#include "daytrender.h"

#include <hirzel/fileutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

#include "data/candle.h"
#include "data/asset.h"
#include "data/tradealgorithm.h"

#include "api/alpacaclient.h"
#include "api/oandaclient.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <filesystem>

using namespace hirzel;
using namespace httplib;

namespace daytrender
{
	DayTrender::DayTrender()
	{
		// NOTE: functions must be called in this order

		// loads the credentials of the clients and creates them
		initClients();
		// loads the asset data and creates assets and their respective algorithms
		initAssets();
		// sets the callbacks for the server
		initServer();
	}

	// destroys clients and assets
	DayTrender::~DayTrender()
	{
		(forex) ? delete forex : warningf("Forex was never initialized!");
		(stocks) ? delete stocks : warningf("Forex was never initialized!");
		(server) ? delete server : warningf("Server was never initialized!");

		int count = 0;
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
			delete p.second;
			count++;
		}
		if(!count) warningf("No algorithms were initialized!");
	}

	// loads clients credentials and creates clients
	void DayTrender::initClients()
	{
		infof("Initializing clients...");
		// loading credentials for apis
		std::string credentialStr = read_string("res/keys.json");
		if (credentialStr.empty())
		{
			//std::cout << "DayTrender::initAssets() : credentialStr is empty!\n";
			errorf("credentialStr is empty!");
			shouldrun = false;
			return;
		}
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

	void DayTrender::loadAlgorithm(const std::string& filename)
	{
		
	}

	// loads asset info, creates assets, and loads algorithm names
	void DayTrender::initAssets()
	{
		infof("Initializing assets...");
		// Loading asset info
		std::string assetStr = read_string("res/assets.json");
		if (assetStr.empty())
		{
			errorf("Failed to load res/assets.json!");
			shouldrun = false;
			return;
		}
		successf("Loaded res/assets.json");
		json assetInfo = json::parse(assetStr);

		assets.resize(ASSET_TYPE_COUNT);



		// creating assets
		for (unsigned int i = 0; i < ASSET_TYPE_COUNT; i++)
		{
			for (json asset : assetInfo[asset_labels[i]])
			{
				unsigned int interval, window;
				std::string ticker, algorithm;

				ticker = asset["ticker"].get<std::string>();
				algorithm = asset["algorithm"].get<std::string>();

				// initializing algorithm
				if(!algorithms[algorithm])
				{
					TradeAlgorithm* algo = new TradeAlgorithm(algorithm);
					if(!algo->isBound())
					{
						delete algo;
						algo = nullptr;
					}
					algorithms[algorithm] = algo;
				}
				
				interval = asset["interval"].get<int>();
				window = asset["window"].get<int>();

				hirzel::printf("Ticker: %s\n", {ticker});

				assets[i].push_back(new Asset(i, forex, algorithms[algorithm], ticker, interval, window));
			}
		}
	}

	void DayTrender::initServer()
	{
		infof("Loading server data...");
		std::string data = read_string("./res/serverinfo.json");
		if(data.empty())
		{
			errorf("Failed to read serverinfo.json!");
			shouldrun = false;
			return;
		}
		json serverInfo = json::parse(data);

		this->ip = serverInfo["ip"].get<std::string>();
		this->port = serverInfo["port"].get<int>();
		this->username = serverInfo["username"].get<std::string>();
		this->password = serverInfo["password"].get<std::string>();
		this->server = new httplib::Server();

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

		serverThread = std::thread(&httplib::Server::listen, server, ip.c_str(), port, 0);
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
		serverThread.join();
		conioThread.join();
	}

	void DayTrender::update()
	{
		infof("Updating assets...");

		for (std::vector<Asset *> vec : assets)
		{
			for (Asset *a : vec)
			{
				a->update();
			}
		}
	}

	void DayTrender::stop()
	{
		if (!running)
		{
			std::cout << "DayTrender has already stopped!" << std::endl;
			return;
		}

		running = false;
		server->stop();
		std::cout << "Shutting down..." << std::endl;
	}

	void DayTrender::scanInput()
	{
		std::string input;

		while (running)
		{
			std::cin >> input;

			if (input == "exit")
			{
				stop();
			}
			// for testing response time of console
			else if (input == "hello")
			{
				hirzel::printf("Hello, Ike~\n");
			}
			else if (input == "time")
			{
				hirzel::printf("unixtime: %d\n", { hirzel::getSeconds() });
			}
			else
			{
				hirzel::printf("Command not found!\n");
			}
		}
	}
} // namespace daytrender
