#pragma once

#include <thread>
#include <vector>
#include <string>
#include <unordered_map>

#define PAPER_BY_DEFAULT
#define BACKTESTING
#define JIT_COMPILE_ALGORITHMS
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

		// REST clients
		OandaClient* forex = nullptr;
		AlpacaClient* stocks = nullptr;

		// server and server info
		std::string ip, username, password;
		unsigned short port = 0;
		httplib::Server* server = nullptr;

		// threads
		std::thread serverThread, conioThread;

		// data containers
		std::vector<std::vector<Asset*>> assets;
		std::unordered_map<std::string, TradeAlgorithm*> algorithms;

		void initClients();
		void initAssets();
		void initServer();
		void loadAlgorithm(const std::string &filename);

		void update();
		void scanInput();
	public:
		DayTrender();
		~DayTrender();

		void start();
		void stop();
	};
}