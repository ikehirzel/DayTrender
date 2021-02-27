/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 ***********************/

#pragma once

#include <string>
#include <vector>

#include "api/portfolio.h"

#define STRATEGY_DIR	"/strategies/"
#define CLIENT_DIR		"/clients/"

namespace daytrender
{
	void init(const std::string& execpath);
	void free();

	void start();
	void stop();

	bool is_running();

	std::vector<std::string> portfolio_names();
	const Portfolio& get_portfolio(int index);
}