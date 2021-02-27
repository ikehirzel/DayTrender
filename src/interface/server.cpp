#include "server.h"

#include "interface.h"
#include "../daytrender.h"
#include "../api/asset.h"
#include <mutex>
#include <string>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <hirzel/fountain.h>

#define JSON_FORMAT	"application/json"
#define TEXT_FORMAT "text/plain"

#define READ_INTERFACE_ON_STARTUP

#ifdef READ_INTERFACE_ON_STARTUP
#include <hirzel/fileutil.h>
#endif

using nlohmann::json;

namespace daytrender
{
	namespace server
	{
		httplib::Server server;
		std::string ip, username, password;
		std::string html
		#ifndef READ_INTERFACE_ON_STARTUP
		{
			#include "webinterface.inc"
		}
		#endif
		;
		
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
				WARNING("Server is already running");
				mtx.unlock();
				return;
			}

			if(ip.empty())
			{
				ERROR("Server has not been initialized properly! Halting execution of server...");
				mtx.unlock();
				return;
			}
			running = true;
			INFO("Starting server @ %s:%d", ip, port);
			mtx.unlock();

			server.listen(ip.c_str(), port);
			SUCCESS("Server has stopped.");
		}

		void stop()
		{
			mtx.lock();
			if(!running)
			{
				WARNING("The server has already been stopped");
				mtx.unlock();
				return;
			}
			running = false;
			server.stop();
			
			mtx.unlock();
		}

		void get_root(const httplib::Request& req,  httplib::Response& res)
		{
			DEBUG("Server GET @ %s", req.path);
			#ifdef READ_INTERFACE_ON_STARTUP
			html = hirzel::file::read_file_as_string("./webinterface.html");
			#endif
			res.set_content(html, "text/html");
		}

		void get_data(const httplib::Request& req,  httplib::Response& res)
		{
			DEBUG("Server GET @ %s", req.path);

			json response;

			// gathering strategy names
			// auto strat_info = strategy_names();
			// if(strat_info.empty())
			// {
			// 	WARNING("No strategy names were received");
			// }
			// else
			// {
			// 	for (int i = 0; i < strat_info.size(); i++)
			// 	{
			// 		response["strategy"][i] = strat_info[i];
			// 	}
			// }
			// gathering asset types and tickers
			// auto asset_data = asset_names();
			// if (asset_data.empty())
			// {
			// 	WARNING("No asset data was received");
			// }
			// else
			// {
			// 	for (int i = 0; i < asset_data.size(); i++)
			// 	{
			// 		response["assets"][i]["ticker"] = asset_data[i].first;
			// 		response["assets"][i]["type"] = asset_data[i].second;
			// 	}
			// }

			// auto client_data = client_names();
			// for (int i = 0; i < client_data.size(); i++)
			// {
			// 	response["types"][i] = client_data[i];
			// }

			// sending json object to client
			res.set_content(response.dump(), JSON_FORMAT);
		}

		void get_accinfo(const httplib::Request& req,  httplib::Response& res)
		{
			DEBUG("Server: GET @ %s", req.path);

			int asset_type;
			AccountInfo accinfo;
			json response;

			asset_type = std::stoi(req.get_param_value("asset_type"));

			const Client* client;// = get_client(asset_type);
			accinfo = client->get_account_info();
			response["balance"] = accinfo.balance();
			response["buying_power"] = accinfo.buying_power();
			response["equity"] = accinfo.equity();

			res.set_content(response.dump(), JSON_FORMAT);
		}

		void get_watch(const httplib::Request& req,  httplib::Response& res)
		{
			DEBUG("Server: GET @ %s", req.path);

			int index = std::stoi(req.get_param_value("index"));
			const Asset* asset;// = get_asset(index);
			const StrategyData& data = asset->data();
			const Client* client;// = asset->client();
			AssetInfo info = client->get_asset_info(asset->ticker());

			json response;
			json& jacc = response["asset"];
			jacc["risk"] = asset->risk();
			jacc["shares"] = info.shares();
			jacc["live"] = asset->is_live();
			jacc["paper"] = false;

			response["interval"] = data.candles().interval();
			response["ticker"] = asset->ticker();

			const CandleSet& c = data.candles();
			int len = asset->data_length();
			int ci = c.size() - len;
			for (unsigned i = 0; i < len; i++)
			{
				response["x"][i] = i;
				response["open"][i] = c[ci].open();
				response["high"][i] = c[ci].high();
				response["low"][i] = c[ci].low();
				response["close"][i] = c[ci].close();
				response["volume"][i] = c[ci].volume();
				ci++;
			}

			//unsigned indi_index = 0;
	
			for (int i = 0; i < data.size(); i++)
			{
				json& indi = response["indicators"][i];
				indi["type"] = data[i].type();
				indi["label"] = data[i].label();
				const Indicator& indicator = data[i];
				for (int j = 0; j < indicator.size(); j++)
				{
					indi["data"][j] = indicator.front(j);
				}
			}
			res.set_content(response.dump(), JSON_FORMAT);
		}

		/*
			TODO:
			Add customizability to the backtest calls
		*/

		void get_backtest(const httplib::Request& req,  httplib::Response& res)
		{
			DEBUG("Server GET @ %s", req.path);
			
			int asset_index, strat_index;
			json response;
			strat_index = std::stoi(req.get_param_value("strategy"));
			asset_index = std::stoi(req.get_param_value("asset"));
			std::string sranges = req.get_param_value("ranges");
			std::vector<PaperAccount> results;
			
			if (sranges.empty())
			{
				results = interface::backtest(strat_index, asset_index, 500.0, false, 5, 155, 10, {});
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
				results = interface::backtest(strat_index, asset_index, 500, false, 5, 155, 10, ranges);
			}
			std::cout << "Got result!\n";

			for (int i = 0; i < results.size(); i++)
			{
				json& curr = response[i];

				curr["buys"] = results[i].long_entrances();
				curr["sells"] = results[i].long_exits();
				curr["interval"] = results[i].interval();

				auto ranges = results[i].ranges();
				for (int j = 0; j < ranges.size(); j++)
				{
					curr["ranges"][j] = ranges[j];
				}

				/*
					TODO :
						Redo all of these assignments to fit with current system
				*/

				curr["elapsedhrs"] = results[i].elapsed_hours();
				curr["initial"] = results[i].principal();
				curr["shares"] = results[i].shares();
				curr["balance"] = results[i].balance();
				curr["equity"] = results[i].equity();
				curr["netreturn"] = results[i].net_return();
				curr["preturn"] = results[i].pct_return();
				curr["hrreturn"] = results[i].net_per_year();
				curr["phrreturn"] = results[i].pct_per_year();
				curr["winrate"] = results[i].long_win_rate();
				curr["bwinrate"] = results[i].long_win_rate();
				curr["swinrate"] = results[i].long_win_rate();
			}

			res.set_content(response.dump(), JSON_FORMAT);
		}

		void get_shutdown(const httplib::Request& req,  httplib::Response& res)
		{
			DEBUG("Server GET @ %s", req.path);
			res.set_content("Shutting down...", TEXT_FORMAT);
			daytrender::stop();
		}
	}
}