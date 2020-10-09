#include "daytrender.h"

#include "util/fileutil.h"
#include "util/sysutil.h"

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

#define DAYTRENDER_NAME "DayTrender"

namespace daytrender
{
	DayTrender::DayTrender()
	{
		l = hirzel::Logger(DAYTRENDER_NAME);
		// NOTE: functions must be called in this order

		// loads the credentials of the clients and creates them
		initClients();
		// binds to the algorithms plugins using the names found in assets.json
		initAlgorithms();
		// loads the asset data and creates assets
		initAssets();
		// sets the callbacks for the server
		initServer();
	}

	// destroys clients and assets
	DayTrender::~DayTrender()
	{
		delete forex;
		delete stocks;
		delete server;

		for (std::vector<Asset *> v : assets)
		{
			for (Asset *asset : v)
			{
				delete asset;
			}
		}

		for (std::pair<std::string, TradeAlgorithm *> p : algorithms)
		{
			delete p.second;
		}
	}

	// loads clients credentials and creates clients
	void DayTrender::initClients()
	{
		l.info("Initializing clients...");
		// loading credentials for apis
		std::string credentialStr = read_string("res/keys.json");
		if (credentialStr.empty())
		{
			//std::cout << "DayTrender::initAssets() : credentialStr is empty!\n";
			l.error("credentialStr is empty!");
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

	void DayTrender::initAlgorithms()
	{
		l.info("Initializing algorithms...");
		std::string algo_dir = "./res/algorithms";

		for (auto &file : std::filesystem::directory_iterator(algo_dir))
		{
			std::string filepath = file.path().string();
			std::string filename = file.path().filename().string();

			TradeAlgorithm *ptr = algorithms[filename];
			if (ptr)
			{

				delete ptr;
			}
			algorithms[filename] = new TradeAlgorithm(filepath);
		}
	}

	// loads asset info, creates assets, and loads algorithm names
	void DayTrender::initAssets()
	{
		l.info("Initializing assets...");
		// Loading asset info
		std::string assetStr = read_string("res/assets.json");
		if (assetStr.empty())
		{
			l.error("assetStr is empty!");
			shouldrun = false;
			return;
		}
		json assetInfo = json::parse(assetStr);

		const char *labels[] = ASSET_LABELS;
		assets.resize(ASSET_TYPE_COUNT);

		// creating assets
		for (unsigned int i = 0; i < ASSET_TYPE_COUNT; i++)
		{
			//std::cout << "Asset[" << i << "] : " <<  assetInfo[labels[i]].dump() << std::endl;

			for (json asset : assetInfo[labels[i]])
			{
				unsigned int interval, window;
				std::string ticker, algorithm;

				ticker = asset["ticker"].get<std::string>();
				algorithm = asset["algorithm"].get<std::string>();
				interval = asset["interval"].get<int>();
				window = asset["window"].get<int>();

				assets[i].push_back(new Asset(i, forex, algorithms[algorithm], ticker, interval, window));
			}
		}
	}

	void DayTrender::initServer()
	{
		l.info("Loading server data...");
		std::string data = read_string("./res/serverinfo.json");
		json serverInfo = json::parse(data);

		this->ip = serverInfo["ip"].get<std::string>();
		this->port = serverInfo["port"].get<int>();
		this->username = serverInfo["username"].get<std::string>();
		this->password = serverInfo["password"].get<std::string>();
		this->server = new httplib::Server();

		l.info("Initializing server...");
		/*
		server->set_mount_point("/", "res/http/");
		
		server->Get("/", [&](const httplib::Request &req, httplib::Response &res)
		{
			std::cout << "GET @ " << req.path << std::endl;
			std::string html = read_file_as_string("res/http/index.html");
			res.set_content(html, "text/html");
		});
		
		server->Get("/data", [&](const Request &req, Response &res) 
		{
			std::cout << "GET @ " << req.path << std::endl;
			
			json response;
			
			//getting all of the assets and sending their tickers
			for(unsigned int j = 0; j < ASSET_TYPE_COUNT; j++)
			{
				std::cout << "label: " << assetLabels[j] << std::endl;
				response["assets"][j]["label"] = assetLabels[j];
				for(unsigned int i = 0; i < assets[j].size(); i++)
				{
					std::cout << assets[j][i]->getTicker() << std::endl;
					response[s("assets")][j][s("list")][i] = value(s(assets[j][i]->getTicker()));
					std::cout << "done" << std::endl;
				}
			}
			std::cout << "Getting algorithms..." << std::endl;
			//getting all of the algorithms in the array and sending
			//their names in their respective indexes 
			for(unsigned int i = 0; i < algorithms.size(); i++)
			{
				response[s("algorithms")][i] = value(s(algorithms[i]->getName()));
			}
			
			res.set_content(u(response.serialize()), "application/json");
		});
		server->Get("/shutdown", [&](const Request &req, Response &res)
		{
			std::cout << "GET @ " << req.path << std::endl;
			
			res.set_content("Shutting down...", "text/plain");
			stop();
		});
		server->Get("/watch", [&](const Request &req, Response &res)
		{
			std::cout << "GET @ " << req.path << std::endl;
			
			unsigned int assetType, interval;
			std::string ticker, responseStr;
			candleset candles;

			assetType = std::stoul(req.get_param_value("assetType"));
			ticker = req.get_param_value("ticker");

			std::cout << "Ticker: " << ticker << ", assetType: " << assetType << std::endl;
			
			Asset* asset;
			
			for(Asset* a : assets[assetType])
			{
				if(a->getTicker() == ticker)
				{
					asset = a;
					break;
				}
			}
			
			if(!asset)
			{
				std::cout << "Asset is nullptr!" << std::endl;
				return;
			}
			asset_data data = asset->getData();
			candles = data.first.second;
			interval = data.first.first;
			
			algorithm_data algodata = data.second;
			
			//the reponse to the server that will be serialized
			value response;
			//the individual variable arrays that will be sent
			value x, open, high, low, close, volume, indicators;
			
			unsigned int index = 0;
			for(indicator_data i : algodata.first)
			{
				//std::cout << "data[" << i << "]: " << indicatorData[i].second.back() << std::endl;
				indicators[index][s("name")] = value(s(i.first));
				for(unsigned int j = 0; j < i.second.size(); j++)
				{
					indicators[index][s("data")][j] = i.second[j];
				}
				index++;
			}
			
			std::cout << "c size: " << candles.size() << std::endl;
			
			for (unsigned int i = 0; i < candles.size(); i++)
			{
				x[i] = i;
				open[i] = candles[i].open;
				high[i] = candles[i].high;
				low[i] = candles[i].low;
				close[i] = candles[i].close;
				volume[i] = candles[i].volume;
			}
			
			response[s("x")] = x;
			response[s("open")] = open;
			response[s("high")] = high;
			response[s("low")] = low;
			response[s("close")] = close;
			response[s("volume")] = volume;
			response[s("indicators")] = indicators;
			response[s("interval")] = interval;
			
			responseStr = u(response.serialize());
			
			res.set_content(responseStr, "application/json");
		});
		server->Get("/backtest", [&](const Request &req, Response &res) 
		{
			std::cout << "GET @ " << req.path << std::endl;
			std::cout << res.body << std::endl;
			//TODO: Implement remote testing
		});
		*/
	}

	void DayTrender::start()
	{
		if (running)
		{
			l.warning("DayTrender has already started!");
			return;
		}

		if (!shouldrun)
		{
			l.fatal("Execution cannot continue!");
			return;
		}

		l.info("Starting DayTrender...");
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
			l.info(msg);
		}
		else
		{
			l.info(msg);
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
		l.info("Updating assets...");

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
				std::cout << "Hello, Ike!" << std::endl;
			}
			else
			{
				std::cout << "Command not found!" << std::endl;
			}
		}
	}
} // namespace daytrender
