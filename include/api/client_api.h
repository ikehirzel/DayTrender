#ifndef DAYTRENDER_CLIENT_API_H
#define DAYTRENDER_CLIENT_API_H

#ifndef MAX_CANDLES
#define MAX_CANDLES
#error MAX_CANDLES must be defined!
#endif

#ifndef KEY_COUNT
#define KEY_COUNT
#error KEY_COUNT must be defined!
#endif

// local includes
#include <api/versions.h>
#include <api/interval.h>
#include <data/account.h>
#include <data/pricehistory.h>
#include <data/position.h>

// standard library
#include <cstdint>

// external libraries
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#define HIRZEL_LOGGER_I
#include <hirzel/logger.h>


using namespace daytrender;
using namespace hirzel;

#define JSON_FORMAT "application/json"

const char *res_err(const httplib::Result& res)
{
	if (!res)
	{
		return "failed to get a response";
	}
	else if (res->status < 200 || res->status > 299)
	{
		std::cout << res->status << ": " << res->body << std::endl;
		return "response status was not okay";
	}
	else if (res->body.empty())
	{
		return "response body was empty";
	}

	return nullptr;
}

// functions for the c api interface
extern "C"
{
	// user defined functions

	// non returning functions
	const char *init(const char** credentials);
	const char *market_order(const char* ticker, double amount);
	const char *set_leverage(uint32_t multiplier);

	// returning functions
	uint32_t secs_till_market_close();
	const char *to_interval(uint32_t interval);
	const char *get_price_history(PriceHistory* out, const char *ticker);
	const char *get_position(Position* out, const char* ticker);
	const char *get_account(Account* out);

	// pre-defined functions
	uint32_t key_count() { return KEY_COUNT; }
	uint32_t max_candles() { return MAX_CANDLES; }
	uint32_t api_version() { return CLIENT_API_VERSION; }

	// used for freeing buffer from main executable
	void free_buffer(char* buffer) { delete buffer; }
}

#endif
