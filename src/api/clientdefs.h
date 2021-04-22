#ifndef CLIENTDEFS_H
#define CLIENTDEFS_H

#define CLIENT_API_VERSION 1

//#ifndef API_VERSION_CHECK

#ifndef MAX_CANDLES
#define MAX_CANDLES
#error MAX_CANDLES must be defined!
#endif

#ifndef KEY_COUNT
#define KEY_COUNT
#error KEY_COUNT must be defined!
#endif

#include <interval.h>
#include <accountinfo.h>
#include <candle.h>
#include <assetinfo.h>

#include <cstdint>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <hirzel/data.h>

using namespace daytrender;
using namespace httplib;
using namespace hirzel;

#define JSON_FORMAT "application/json"

const char* _error;

bool res_ok(const Result& res)
{
	if (!res)
	{
		error = "failed to get a response";
		return false;
	}
	else if (res->status < 200 || res->status > 299)
	{
		error = "response status was not okay: " + std::to_string(res->status);
		if (!res->body.empty())
		{
			error += ": " + res->body;
		}
		return false;
	}
	else if (res->body.empty())
	{
		error = "response body was empty";
		return false;
	}

	return true;
}

Candle* get_candles(const char* ticker, uint32_t interval, uint32_t count);
AccountInfo* get_account_info();
AssetInfo* get_asset_info(const char* ticker);

extern "C"
{
	// non-returning functions
	bool init(const char** credentials, char n);
	bool market_order(const char* ticker, double amount);
	bool close_all_positions();
	bool set_leverage(uint32_t numerator);

	// protocol functions

	/**
	 *  Protocol:
	 * 		
	 */
	const double* abi_get_candles(const char* ticker, uint32_t interval, uint32_t count)
	{

		// call user defined function and return if failed
		Candle* candles = get_candles(ticker, interval, count);
		if (!candles) return nullptr;

		double* data = new double[count];
		for (size_t i = 0; i < count; ++i)
		{
			*data++ = candles[i].open();
			*data++ = candles[i].high();
			*data++ = candles[i].low();
			*data++ = candles[i].close();
			*data++ = candles[i].volume();
		}
		// null byte at end so size does not need to be sent

		return serial;
	}

	const char* abi_get_account_info()
	{
		// account info data
		double balance = 0.0;
		double buying_power = 0.0;
		double margin_used = 0.0;
		double equity = 0.0;
		uint32_t leverage = 0;
		bool shorting_enabled = false;

		// confirming that the function runs correctly
		if (!get_account_info(balance, buying_power, margin_used,
			equity, leverage, shorting_enabled)) return nullptr;

		// creating memory buffer
		size_t osize = 4 * sizeof(double) + sizeof(uint32_t) + sizeof(bool) + sizeof(char);
		char* serial = new char[osize];

		// moving doubles into buffer
		double* dp = (double*)serial;
		dp[0] = balance;
		dp[1] = buying_power;
		dp[2] = margin_used;
		dp[3] = equity;

		// moving integer into buffer
		uint32_t* uip = (uint32_t*)(serial + 4 * sizeof(double));
		uip[0] = leverage;

		//moving bool into buffer
		bool* bp = (bool*)(serial + 4 * sizeof(double) + sizeof(uint32_t));
		bp[0] = shorting_enabled;

		// null byte at end so size doesn't need to be sent
		serial[osize - 1] = 0;

		return serial;
	}

	const char* abi_get_asset_info(const char* ticker)
	{

	}

	const char* to_interval(uint32_t interval);

	// returning functions
	uint32_t secs_till_market_close();

	// const functions
	uint32_t key_count() { return KEY_COUNT; }
	uint32_t max_candles() { return MAX_CANDLES; }
	uint32_t api_version() { return CLIENT_API_VERSION; }
	const char* error() { return _error; }

	// used for freeing buffer from main executable
	void free(const char* buffer)
	{
		delete buffer;
	}
}
#endif