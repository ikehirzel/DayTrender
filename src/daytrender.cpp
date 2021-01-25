#include "daytrender.h"

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

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

	bool getline_async(std::istream& is, std::string& str, char delim = '\n')
	{
		std::string lineSoFar;
		char inChar;
		int charsRead = 0;
		bool lineRead = false;
		str = "";

		do {
			charsRead = is.readsome(&inChar, 1);
			if (charsRead == 1) {
				// if the delimiter is read then return the string so far
				if (inChar < 32) {
					str = lineSoFar;
					lineSoFar = "";
					lineRead = true;
				} else {  // otherwise add it to the string so far
					lineSoFar.append(1, inChar);
				}
			}
		} while (charsRead != 0 && !lineRead);

		return lineRead;
	}

	void init(const std::string& execpath)
	{
		// while (true)
		// {
		// 	std::string input;
		// 	if (std::cin.rdbuf()->in_avail())
		// 	{
		// 		std::getline(std::cin, input);
		// 		std::cout << "input: " << input << std::endl;
		// 	}
		// 	else
		// 	{
		// 		std::cout << "Available: " << std::cin.rdbuf()->in_avail() << std::endl;
		// 	}
		// 	sys::thread_sleep(1000);
		// }

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
			std::string label = client_json["label"].get<std::string>();
			std::string filename = client_json["filename"].get<std::string>();
			double history_length = client_json["history_length"].get<double>();
			double max_loss = client_json["max_loss"].get<double>();
			int leverage = client_json["leverage"].get<int>();
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

			Client* cli = new Client(label, dtdir + CLIENTS_DIR + filename, args, risk, max_loss, history_length, leverage);
		
			if (!cli->bound())
			{
				errorf("%s client %s failed to bind!", label, filename);
				delete cli;
				continue;
			}

			if (!cli->is_live())
			{
				errorf("%s assets cannot be initialized!", label);
				delete cli;
				continue;
			}

			clients.push_back(cli);

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

		running = true;
		mtx.unlock();

		std::thread shell_thread(shell::get_input);
		std::thread server_thread(server::start);
		shell_thread.detach();
		infof("Starting DayTrender");

		while (running)
		{
			int live_assets = 0;
			// check if every asset needs to update every 500ms, update each asset as needed 
			for (Asset* asset : assets)
			{
				if (asset->is_live())
				{
					if (asset->should_update()) asset->update();
					live_assets++;
				}
			}
			if (live_assets == 0)
			{
				warningf("No assets were live for updating. Excecution cannot continue.");
				stop();
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
			out[i] = algorithms[i]->get_filename(); 
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