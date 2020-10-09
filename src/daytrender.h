#pragma once

#include <hlog.h>
#include <thread>
#include <vector>
#include <string>
#include <map>

#define PAPER_BY_DEFAULT
#define BACKTESTING

namespace httplib
{
	class Server;
}

namespace daytrender
{
	class TradeAlgorithm;
	class AlpacaClient;
	class OandaClient;
	class Asset;

	class DayTrender
	{
	private:
		bool running = false, shouldrun = true;
		hirzel::Logger l;
		// rest clients
		OandaClient* forex;
		AlpacaClient* stocks;

		// server and server info
		std::string ip, username, password;
		unsigned short port = 0;
		httplib::Server* server;

		// threads
		std::thread serverThread, conioThread;

		// data containers
		std::vector<std::vector<Asset*>> assets;
		std::map<std::string, TradeAlgorithm*> algorithms;

		void initClients();
		void initAssets();
		void initAlgorithms();
		void initServer();

		void update();
		void scanInput();
	public:
		DayTrender();
		~DayTrender();

		void start();
		void stop();
	};
}