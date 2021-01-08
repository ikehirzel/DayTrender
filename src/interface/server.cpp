#include "server.h"

#include "../daytrender.h"
#include "../data/asset.h"

#include <mutex>
#include <string>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <hirzel/fileutil.h>
#include <hirzel/fountain.h>

#define SERVER_CONFIG_FILENAME CONFIG_FOLDER "serverinfo.json"

using nlohmann::json;

namespace daytrender
{
	namespace server
	{
		httplib::Server server;
		std::string ip, username, password;
		unsigned short port;
		bool running = false;
		std::mutex mtx;

		void get_root(const httplib::Request& req,  httplib::Response& res);
		void get_data(const httplib::Request& req,  httplib::Response& res);
		void get_shutdown(const httplib::Request& req,  httplib::Response& res);
		void get_watch(const httplib::Request& req,  httplib::Response& res);
		void get_backtest(const httplib::Request& req,  httplib::Response& res);

		bool init(const std::string& dir)
		{
			std::string data;

			infof("Loading server data...");

			data = hirzel::file::read_file_as_string(dir + SERVER_CONFIG_FILENAME);

			if(data.empty())
			{
				fatalf("Failed to read ." SERVER_CONFIG_FILENAME "!");
				return false;
			}

			successf("Successfully loaded ." SERVER_CONFIG_FILENAME);

			json serverInfo = json::parse(data);
			ip = serverInfo["ip"].get<std::string>();
			port = serverInfo["port"].get<unsigned short>();
			username = serverInfo["username"].get<std::string>();
			password = serverInfo["password"].get<std::string>();

			server.set_mount_point("/", std::string(dir + RESOURCES_FOLDER "http").c_str());

			server.Get("/", get_root);
			server.Get("/data", get_data);
			server.Get("/shutdown", get_shutdown);
			server.Get("/watch", get_watch);
			server.Get("/backtest", get_backtest);
			return true;
		}

		void start()
		{
			mtx.lock();
			if (running)
			{
				warningf("Server is already running");
				mtx.unlock();
				return;
			}

			if(ip.empty())
			{
				errorf("Server has not been initialized properly! Halting execution of server...");
				mtx.unlock();
				return;
			}
			running = true;
			infof("Starting server @ %s:%d", ip, port);
			mtx.unlock();

			server.listen(ip.c_str(), port);
		}

		void stop()
		{
			mtx.lock();
			if(!running)
			{
				warningf("The server has already been stopped");
				mtx.unlock();
				return;
			}
			infof("Shutting down server...");
			running = false;
			server.stop();
			mtx.unlock();
		}

		void get_root(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server GET @ %s", req.path);
			warningf("'/' callback is not yet implemented");
			// TODO: Implement /
		}

		void get_data(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server GET @ %s", req.path);

			json response;

			// gathering algorithm names
			auto algo_info = getAlgoInfo();
			if(algo_info.empty())
			{
				warningf("No algorithm names were received");
			}
			else
			{
				for (int i = 0; i < algo_info.size(); i++)
				{
					response["algorithms"][i]["filename"] = algo_info[i];
				}
			}

			// gathering asset types and tickers
			auto asset_data = getAssetInfo();
			if (asset_data.empty())
			{
				warningf("No asset data was received");
			}
			else
			{
				for (int i = 0; i < asset_data.size(); i++)
				{
					response["assets"][i]["ticker"] = asset_data[i].first;
					response["assets"][i]["type"] = asset_data[i].second;
				}
			}

			for (int i = 0; i < ASSET_TYPE_COUNT; i++)
			{
				response["types"][i] = asset_labels[i];
			}

			// sending json object to client
			res.set_content(response.dump(), "application/json");
		}

		void get_watch(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server: GET @ %s", req.path);

			int index;
			std::string ticker;
			algorithm_data data;
			json response;
			const Asset* asset;

			index = std::stoi(req.get_param_value("index"));
			asset = getAsset(index);
			data = asset->getData();

			response["interval"] = data.candles.interval;
			response["ticker"] = asset->getTicker();

			const candleset& c = data.candles;
			for (unsigned i = 0; i < c.size; i++)
			{
				response["x"][i] = i;
				response["open"][i] = c[i].open;
				response["high"][i] = c[i].high;
				response["low"][i] = c[i].low;
				response["close"][i] = c[i].close;
				response["volume"][i] = c[i].volume;
			}

			unsigned indi_index = 0;
			const std::vector<indicator_data>& dataset = data.dataset;
	
			for (int i = 0; i < dataset.size(); i++)
			{
				json& indi = response["indicators"][i];
				indi["type"] = dataset[i].type;
				indi["label"] = dataset[i].label;
				for (int j = 0; j < dataset[i].data.size(); j++)
				{
					indi["data"][j] = dataset[i].data[j];
				}
			}
			res.set_content(response.dump(), "application/json");
		}

		void get_backtest(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server GET @ %s", req.path);
			
			
		}

		void get_shutdown(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server GET @ %s", req.path);
			res.set_content("Shutting down...", "text/plain");
			daytrender::stop();
		}
	}
}