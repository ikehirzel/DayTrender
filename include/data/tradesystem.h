/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 ***********************/

#ifndef DAYTRENDER_TRADESYSTEM_H
#define DAYTRENDER_TRADESYSTEM_H

// local includes
#include <data/portfolio.h>

// standard library
#include <string>
#include <vector>
#include <mutex>

namespace daytrender
{
	class TradeSystem
	{
	private:
		bool _running = false;
		bool _initialized = false;
		std::mutex _mtx;
		std::vector<Portfolio> _portfolios;

		bool init(const std::string& dir);

	public:
		TradeSystem(const std::string& dir);

		void start();

		/**
		 * Changes running to false. This will interrupt the program loop.
		 */
		void stop();

		/**
		 * @return	list of portfolios
		 */
		inline const std::vector<Portfolio>& get_portfolios() const
		{
			return _portfolios;
		}

		inline bool is_running() const { return _running; }
		inline bool is_initialized() const { return _initialized; }
		Portfolio *get_portfolio(const std::string& label);
	};
}

#endif // DAYTRENDER_TRADESYSTEM_H
