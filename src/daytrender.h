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
	/**
	 * Changes running to false. This will interrupt the program loop.
	 */
	void stop();

	/**
	 * @return	list of portfolios
	 */
	const std::vector<Portfolio>& get_portfolios();
}

#endif // DAYTRENDER_H