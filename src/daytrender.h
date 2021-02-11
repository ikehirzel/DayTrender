/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 ***********************/

#pragma once

#include <string>
#include <vector>

#include "api/asset.h"
#include "api/client.h"
#include "api/paperaccount.h"

#define STRATEGY_DIR	"/strategies/"
#define CLIENT_DIR		"/clients/"

namespace daytrender
{
	void init(const std::string& execpath);
	void free();

	void start();
	void stop();

	bool is_running();

	std::vector<std::string> client_names();
	std::vector<std::string> strategy_names();
	std::vector<std::pair<std::string, int>> asset_names();

	const Asset* get_asset(int index);
	const Client* get_client(int type);
	const Strategy* get_strategy(int index);
}