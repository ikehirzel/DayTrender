#include "server.h"

#include "../daytrender.h"
#include "../data/asset.h"

#include <mutex>
#include <string>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <hirzel/fileutil.h>
#include <hirzel/fountain.h>

#define JSON_FORMAT	"application/json"
#define TEXT_FORMAT "text/plain"

using nlohmann::json;

namespace daytrender
{
	namespace server
	{
		httplib::Server server;
		std::string ip, username, password;
		std::string html{
			#include "webinterface.inc"
		};
		unsigned short port;
		bool running = false;
		std::mutex mtx;

		void get_root(const httplib::Request& req,  httplib::Response& res);
		void get_data(const httplib::Request& req,  httplib::Response& res);
		void get_shutdown(const httplib::Request& req,  httplib::Response& res);
		void get_watch(const httplib::Request& req,  httplib::Response& res);
		void get_backtest(const httplib::Request& req,  httplib::Response& res);
		void get_accinfo(const httplib::Request& req,  httplib::Response& res);

		bool init(const json& config, const std::string& dir)
		{
			ip = config["ip"].get<std::string>();
			port = config["port"].get<unsigned short>();
			username = config["username"].get<std::string>();
			password = config["password"].get<std::string>();

			server.Get("/", get_root);
			server.Get("/data", get_data);
			server.Get("/shutdown", get_shutdown);
			server.Get("/watch", get_watch);
			server.Get("/backtest", get_backtest);
			server.Get("/accinfo", get_accinfo);
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

			res.set_content(html, "text/html");
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
					response["algorithms"][i] = algo_info[i];
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

			auto client_data = getClientInfo();
			for (int i = 0; i < client_data.size(); i++)
			{
				response["types"][i] = client_data[i];
			}

			// sending json object to client
			res.set_content(response.dump(), JSON_FORMAT);
		}

		void get_accinfo(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server: GET @ %s", req.path);

			int asset_type;
			account_info accinfo;
			;
			json response;

			asset_type = std::stoi(req.get_param_value("asset_type"));

			const Client* client = getClient(asset_type);
			accinfo = client->get_account_info();
			response["balance"] = accinfo.balance;
			response["buying_power"] = accinfo.buying_power;
			response["equity"] = accinfo.equity;

			res.set_content(response.dump(), JSON_FORMAT);
		}

		void get_watch(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server: GET @ %s", req.path);

			int index;
			std::string ticker;
			algorithm_data data;
			asset_info ainfo;
			json response;

			index = std::stoi(req.get_param_value("index"));
			const Asset* asset = getAsset(index);
			data = asset->getData();
			ainfo = asset->getAssetInfo();

			json& jacc = response["asset"];
			jacc["risk"] = ainfo.risk;
			jacc["shares"] = ainfo.shares;
			jacc["live"] = ainfo.live;
			jacc["paper"] = ainfo.paper;

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
			const indicator* dataset = data.dataset;
	
			for (int i = 0; i < data.ranges_size - 1; i++)
			{
				json& indi = response["indicators"][i];
				indi["type"] = dataset[i].type;
				indi["label"] = dataset[i].label;
				for (int j = 0; j < dataset[i].size; j++)
				{
					indi["data"][j] = dataset[i].front(j);
				}
			}
			res.set_content(response.dump(), JSON_FORMAT);
		}

		void get_backtest(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server GET @ %s", req.path);
			
			int asset_index, algo_index;
			json response;
			algo_index = std::stoi(req.get_param_value("algorithm"));
			asset_index = std::stoi(req.get_param_value("asset"));
			std::string sranges = req.get_param_value("ranges");
			std::vector<PaperAccount> results;
			
			if (sranges.empty())
			{
				results = daytrender::backtest(algo_index, asset_index, {});
			}
			else
			{	
				std::cout << "ranges were gotten!\n";
				std::vector<int> ranges;
				std::string tmp;

				for (int i = 0; i < sranges.size(); i++)
				{
					if (sranges[i] == ',' || sranges[i] == ' ')
					{
						if (!tmp.empty())
						{
							ranges.push_back(std::stoi(tmp));
							tmp.clear();
						}
						continue;
					}
					tmp += sranges[i];
				}

				if (!tmp.empty())
				{
					ranges.push_back(std::stoi(tmp));
				}

				std::cout << "Ranges: " << ranges.size() << std::endl;
				results = daytrender::backtest(algo_index, asset_index, ranges);
			}
			std::cout << "Got result!\n";

			for (int i = 0; i < results.size(); i++)
			{
				json& curr = response[i];

				curr["buys"] = results[i].getBuys();
				curr["sells"] = results[i].getSells();
				curr["interval"] = results[i].getInterval();

				auto ranges = results[i].getRanges();
				for (int j = 0; j < ranges.size(); j++)
				{
					curr["ranges"][j] = ranges[j];
				}

				curr["elapsedhrs"] = results[i].elapsedHours();
				curr["initial"] = results[i].getInitial();
				curr["shares"] = results[i].getShares();
				curr["balance"] = results[i].getBalance();
				curr["equity"] = results[i].equity();
				curr["netreturn"] = results[i].netReturn();
				curr["preturn"] = results[i].percentReturn();
				curr["hrreturn"] = results[i].avgHourNetReturn();
				curr["phrreturn"] = results[i].avgHourPercentReturn();
				curr["winrate"] = results[i].winRate();
				curr["bwinrate"] = results[i].buyWinRate();
				curr["swinrate"] = results[i].sellWinRate();
			}

			res.set_content(response.dump(), JSON_FORMAT);
		}

		void get_shutdown(const httplib::Request& req,  httplib::Response& res)
		{
			debugf("Server GET @ %s", req.path);
			res.set_content("Shutting down...", TEXT_FORMAT);
			daytrender::stop();
		}
	}
}