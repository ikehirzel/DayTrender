/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 ***********************/

#ifndef DAYTRENDER_H
#define DAYTRENDER_H

#include <string>
#include <vector>

#include "api/portfolio.h"

namespace daytrender
{	
	void init(const std::string& execpath);
	void start();
	void stop();

	const Portfolio& get_portfolio(int index);
	std::vector<std::string> portfolio_names();

}

#endif // DAYTRENDER_H