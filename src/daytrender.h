/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 * 
 ***********************/

#pragma once

#include <string>
#include <vector>

#include "data/paperaccount.h"

#include "data/asset.h"
#include "api/client.h"

#define ALGORITHM_DIR	"/algorithms/"
#define CLIENTS_DIR		"/clients/"

namespace daytrender
{
	std::vector<PaperAccount> backtest(int algo_index, int asset_index, const std::vector<int>& ranges);

	void init(const std::string& execpath);
	void free();

	void start();
	void stop();

	bool is_running();

	std::vector<std::string> get_client_info();
	std::vector<std::string> get_algo_info();
	std::vector<std::pair<std::string, int>> get_asset_info();
	const Asset* get_asset(int index);
	const Client* get_client(int type);
}