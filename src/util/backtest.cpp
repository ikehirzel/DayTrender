#include <interface/backtest.h>

// standard libarary
// #include <future>
// #include <chrono>

// external libraries
#include <hirzel/logger.h>

/*
	CHANGING THE MIN BACKTEST RANGE CAUSE IT TO NOT CRASH
	KNOWN CRASHES AT MIN = 2
*/

namespace daytrender
{
	bool backtest_permutation(PaperAccount& acc, const Asset& asset,
		const PriceHistory& candles, const Strategy *strat,
		const std::vector<unsigned>& ranges)
	{
		for (long i = 0; i < candles.size() - asset.candle_count(); i++)
		{
			PriceHistory slice = candles.slice(i, asset.candle_count());

			acc.update_price(slice.back().close());
			Result<Chart> res = strat->execute(slice, asset.ranges());

			if (!res)
			{
				ERROR("Backtest: Strategy: %s", res.error());
				return false;
			}

			Chart data = res.get();

			bool success = true;
			switch (data.action())
			{
			case NOTHING:
				break;
			case ENTER_LONG:
				success = acc.enter_long();
				break;
			case EXIT_LONG:
				success = acc.exit_long();
				break;
			case ENTER_SHORT:
				success = acc.enter_short();
			case EXIT_SHORT:
				success = acc.exit_short();
			case ERROR:
				return false;
			default:
				ERROR("Invalid action received from strategy");
				return false;
			}
			if (!success)
			{
				
			}
		}
		
		if (!acc.close_position()) return false;

		return true;
	}

	// bool backtest_interval(PaperAccount* best, const Asset* asset, const Strategy* strat,
	// 	int interval, long long permutations, double principal, double minimum, double fee, bool shorting_enabled,
	// 	int leverage, int min_range, int max_range, int granularity, const std::vector<int>& start_ranges)
	// {
	// 	std::vector<int> curr_ranges = start_ranges;
	// 	const Client* client;// = asset->client();
	// 	CandleSet candles = client->get_candles(asset->ticker(), interval, 0, 0);
		
	// 	*best = PaperAccount(principal, leverage, fee, minimum, candles.front().open(),
	// 		shorting_enabled, interval, curr_ranges);

	// 	for (int i = 0; i < permutations; i++)
	// 	{
	// 		// storing activity and performance data
	// 		PaperAccount acc(principal, leverage, fee, minimum, candles.front().open(),
	// 			shorting_enabled, interval, curr_ranges);

	// 		backtest_permutation(acc, candles, strat, curr_ranges);

	// 		if (acc.equity() > best->equity())
	// 		{
	// 			*best = acc;
	// 		}

	// 		// incrementing the permutation

	// 		int pos = 0;
	// 		curr_ranges[pos] += granularity;
	// 		while (curr_ranges[pos] > max_range)
	// 		{
	// 			curr_ranges[pos] = min_range;
	// 			pos++;
	// 			curr_ranges[pos] += granularity;
	// 		}
	// 	}
	// 	return true;
	// }

	// PaperAccount backtest(const Asset* asset)
	// {
	// 	const Client* client;// = asset->client();
	// 	const Strategy& strategy = asset->strategy();

	// 	Account account_info;// = asset->client()->get_account_info();
	// 	Position asset_info;// = asset->client()->get_asset_info(asset->ticker());
	// 	CandleSet candles = client->get_candles(asset->ticker(), asset->interval(), 0, 0);
	// 	PaperAccount account
	// 	{
	// 		account_info.balance(),
	// 		account_info.leverage(),
	// 		asset_info.fee(),
	// 		asset_info.minimum(),
	// 		candles.front().open(),
	// 		account_info.shorting_enabled(),
	// 		asset->interval(),
	// 		asset->ranges()
	// 	};
		
	// 	//backtest_permutation(account, candles, strategy, asset->ranges());

	// 	return account;
	// }

	// std::vector<PaperAccount> backtest(int strat_index, int asset_index, double principal,
	// 	bool shorting_enabled, int min_range, int max_range, int granularity,
	// 	const std::vector<int>& test_ranges)
	// {
	// 	auto t0 = std::chrono::system_clock::now();
	// 	const Asset* asset;// = get_asset(asset_index);
	// 	const Client* client;// = asset->client();
	// 	const Strategy* strat;// = get_strategy(strat_index);

	// 	// storing important info
	// 	std::vector<int> intervals = client->backtest_intervals();

	// 	INFO("Backtesting '%s' with strategy '%s'...", asset->ticker(), strat->filename());

	// 	// calculating lops
	// 	std::vector<int> start_ranges;
	// 	long long permutations = 1;
	// 	int possible_vals = ((max_range + 1) - min_range) / granularity;

	// 	// if no ranges are passed in
	// 	if (test_ranges.empty())
	// 	{
	// 		// setting default ranges
	// 		start_ranges.resize(strat->indicator_count(), min_range);

	// 		// calculate the amount of permutations of ranges
	// 		for (int i = 0; i < strat->indicator_count(); i++) permutations *= possible_vals;
	// 	}
	// 	else
	// 	{
	// 		// setting default ranges to the given ones
	// 		start_ranges = test_ranges;

	// 		// verify ranges size
	// 		if (start_ranges.size() >  strat->indicator_count())
	// 		{
	// 			ERROR("Backtest: Passed in %d ranges but %d were expected! Resizing ranges...", start_ranges.size(), strat->indicator_count());
	// 			start_ranges.resize(strat->indicator_count());
	// 		}
	// 		else if (start_ranges.size() < strat->indicator_count())
	// 		{
	// 			ERROR("Backtest: Passed in %d ranges but %d were expected! Execution cannot continue.", start_ranges.size(), strat->indicator_count());
	// 			return {};
	// 		}

	// 		// verify minimums and maximums
	// 		for (int i = 0; i < start_ranges.size(); i++)
	// 		{
	// 			if (start_ranges[i] < min_range)
	// 			{
	// 				start_ranges[i] = min_range;
	// 				WARNING("test_ranges[%d] was less than the minimum (%d). Readjusting...", start_ranges[i], min_range);
	// 			}
	// 			else if (start_ranges[i] > max_range)
	// 			{
	// 				start_ranges[i] = max_range;
	// 				WARNING("test_ranges[%d] was greater than the maximum (%d). Readjusting...", start_ranges[i], max_range);
	// 			}
	// 		}
	// 	}

	// 	Account acct = client->get_account_info();
	// 	Position info = client->get_asset_info(asset->ticker());
	// 	std::vector<PaperAccount> out(intervals.size());
	// 	std::vector<std::future<bool>> threads(intervals.size());
	// 	// for every intervals
	// 	for (int i = 0; i < intervals.size(); i++)
	// 	{
	// 		threads[i] = std::async(std::launch::async, backtest_interval, &out[i], asset,
	// 		strat, intervals[i], permutations, principal, info.minimum(), info.fee(), shorting_enabled, acct.leverage(),
	// 		min_range, max_range, granularity, start_ranges);
			
	// 	}

	// 	for (int i = 0; i < threads.size(); i++)
	// 	{
	// 		if (threads[i].get())
	// 		{
	// 			SUCCESS("Backtest %d succeeded", i + 1);
	// 		}
	// 		else
	// 		{
	// 			ERROR("Backtest %d failed", i + 1);
	// 		}
			
	// 	}
	// 	auto t = std::chrono::system_clock::now();
	// 	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0);
	// 	double secs = ms.count() / 1000.0;

	// 	SUCCESS("Backtesting of %s has completed in %fs", asset->ticker(), secs);

	// 	return out;
	// }

	// std::vector<PaperAccount> backtest(Client* client, const Strategy* strat, const std::vector<int>& ranges)
	// {
	// 	CandleSet candles = client->get_candles("EUR_USD", 300, 0, 0);
	// 	PaperAccount acc0(500, 1, 0.0001, 1.0, candles.front().open(), false, 300, ranges);
	// 	PaperAccount acc1(500, 10, 0.0001, 1.0, candles.front().open(), false, 300, ranges);

	// 	backtest_permutation(acc0, candles, strat, ranges);
	// 	backtest_permutation(acc1, candles, strat, ranges);

	// 	return { acc0, acc1 };
	// }

	// current interval, current ranges
	Result<PaperAccount> backtest_asset(const Client *client,
		const Asset& asset, const Strategy *strategy)
	{
		PaperAccount out;

		Result<PriceHistory> res = client->get_price_history(asset.ticker(),
			asset.interval(), 0);

		if (!res)
		{
			ERROR(res.error());
			return NULL;
		}

		// if (!backtest_permutation(out, 
		return out;
	}
}