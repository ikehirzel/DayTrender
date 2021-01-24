/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 ***********************/

#pragma once

#include <string>
#include <vector>


#include "data/asset.h"
#include "api/client.h"
#include "api/paperaccount.h"

#define ALGORITHM_DIR	"/algorithms/"
#define CLIENTS_DIR		"/clients/"

namespace daytrender
{
	void init(const std::string& execpath);
	void free();

	void start();
	void stop();

	bool is_running();

	std::vector<std::string> client_names();
	std::vector<std::string> algorithm_names();
	std::vector<std::pair<std::string, int>> asset_names();

	const Asset* get_asset(int index);
	Client* get_client(int type);
	const Algorithm* get_algorithm(int index);
}