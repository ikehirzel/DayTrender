/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 * 
 ***********************/

#pragma once

#include <string>

#define PAPER_BY_DEFAULT
#define BACKTESTING
#define JIT_COMPILE_ALGORITHMS

namespace daytrender
{
	void init(const std::string& execpath);
	void free();

	void start();
	void stop();
}