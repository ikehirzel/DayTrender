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
#include "data/tradealgorithm.h"

#define PAPER_BY_DEFAULT
#define BACKTESTING
#define JIT_COMPILE_ALGORITHMS

#define CONFIG_FOLDER			"/config/"
#define RESOURCES_FOLDER		"/resources/"
#define SCRIPT_FOLDER			"/algorithms/"
#define ALGORITHM_BIN_FOLDER	RESOURCES_FOLDER "bin/"
#define HTML_FOLDER				RESOURCES_FOLDER "html/"

namespace daytrender
{
	bool buildAlgorithm(const std::string& filename, bool print);
	std::vector<PaperAccount> backtest(int algo_index, int asset_index);
	void init(const std::string& execpath);
	void free();

	void start();
	void stop();

	bool isRunning();

	std::vector<std::string> getAlgoInfo();
	std::vector<std::pair<std::string, int>> getAssetInfo();
	const Asset* getAsset(int index);
}