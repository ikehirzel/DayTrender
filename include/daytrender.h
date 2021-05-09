/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 ***********************/

#ifndef DAYTRENDER_H
#define DAYTRENDER_H

// local includes
#include <data/portfolio.h>

// standard library
#include <string>
#include <vector>


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