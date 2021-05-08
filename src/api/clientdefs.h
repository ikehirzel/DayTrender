#ifndef CLIENTDEFS_H
#define CLIENTDEFS_H

#include <api_versions.h>

#ifndef MAX_CANDLES
#define MAX_CANDLES
#error MAX_CANDLES must be defined!
#endif

#ifndef KEY_COUNT
#define KEY_COUNT
#error KEY_COUNT must be defined!
#endif

// Daytrender files
#include <interval.h>
#include <accountinfo.h>
#include <candle.h>
#include <assetinfo.h>
#include <api_versions.h>

// for guaranteed sizes
#include <cstdint>

// external libraries
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <hirzel/data.h>

using namespace daytrender;
using namespace hirzel;

#define JSON_FORMAT "application/json"

const char *error;

namespace hirzel
{
	template <typename T>
	class Result
	{
	private:
		bool _ok = false;
		T _value;
		const char *_error = nullptr;

	public:
		inline Result(T value)
		{
			_ok = true;
			_value = value;
		}

		inline Result(const char *error)
		{
			_ok = false;
			_error = error;
		}

		inline T get()
		{
			if (_ok) return _value;
			return {};
		}

		inline const char* err()
		{
			if (!_ok) return _error;
			return nullptr;
		}
		inline bool ok() { return _ok; }
	};
}
const char *res_err(const httplib::Result& res)
{
	if (!res)
	{
		return "failed to get a response";
	}
	else if (res->status < 200 || res->status > 299)
	{
		return "response status was not okay";
	}
	else if (res->body.empty())
	{
		return "response body was empty";
	}

	return nullptr;
}

// functions that must be defined by user
Result<Candle*> get_candles(const char* ticker, uint32_t interval, uint32_t count);
Result<AccountInfo> get_account_info();
Result<AssetInfo> get_asset_info(const char* ticker);

// functions for the c api interface
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
	double* abi_get_candles(const char* ticker, uint32_t interval, uint32_t count)
	{
		Result<Candle*> res = get_candles(ticker, interval, count);

		if (!res.ok())
		{
			error = res.err();
			return nullptr;
		}

		// call user defined function and return if failed
		Candle* candles = res.get();

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

		return data;
	}

	char* abi_get_account_info()
	{
		// account info data
		bool shorting_enabled = false;

		Result<AccountInfo> res = get_account_info();

		if (!res.ok())
		{
			error = res.err();
			return nullptr;
		}

		AccountInfo info = res.get();

		// creating memory buffer
		size_t osize = 4 * sizeof(double) + sizeof(uint32_t) + sizeof(bool);
		char* serial = new char[osize];
		
		// moving doubles into buffer
		double* dp = (double*)serial;
		dp[0] = info.balance();
		dp[1] = info.buying_power();
		dp[2] = info.margin_used();
		dp[3] = info.equity();

		// moving integer into buffer
		uint32_t* uip = (uint32_t*)(serial + 4 * sizeof(double));
		uip[0] = info.leverage();

		//moving bool into buffer
		bool* bp = (bool*)(serial + 4 * sizeof(double) + sizeof(uint32_t));
		bp[0] = info.shorting_enabled();

		return serial;
	}

	char* abi_get_asset_info(const char* ticker)
	{
		return nullptr;
	}

	const char* to_interval(uint32_t interval);

	// returning functions
	uint32_t secs_till_market_close();

	// const functions
	uint32_t key_count() { return KEY_COUNT; }
	uint32_t max_candles() { return MAX_CANDLES; }
	uint32_t api_version() { return CLIENT_API_VERSION; }
	const char* get_error() { return error; }

	// used for freeing buffer from main executable
	void free_buffer(const char* buffer)
	{
		delete buffer;
	}
}

#endif
