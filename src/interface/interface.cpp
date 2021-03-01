#include "interface.h"

#include "../daytrender.h"

#include <hirzel/fountain.h>

#include <future>
#include <chrono>

/*
	CHANGING THE MIN BACKTEST RANGE CAUSE IT TO NOT CRASH
	KNOWN CRASHES AT MIN = 2
*/

namespace daytrender
{
	namespace interface
	{
		bool backtest_permutation(PaperAccount& acc, CandleSet& candles, const Strategy* strat,
			const std::vector<int>& ranges)
		{
			// calculating size of candles
			int candle_count = 0;
			if (ranges.size() > 1)
			{
				for (int j = 1; j < ranges.size(); j++)
				{
					if (ranges[j] > candle_count) candle_count = ranges[j];
				}
			}
			candle_count += ranges[0];

			for (long i = 0; i < candles.size() - candle_count; i++)
			{
				CandleSet slice = candles.slice(i, candle_count, strat->data_length());

				if (slice.error())
				{
					ERROR("Backtest: CandleSet: %s", slice.error());
					return false;
				}
				
				acc.update_price(slice.back().close());
				StrategyData data = strat->execute(slice, ranges);

				if (data.error())
				{
					ERROR("Backtest: Strategy: %s", data.error());
					return false;
				}

				if (!acc.handle_action(data.action()))
				{
					ERROR("Backtest: Account: %s", acc.error());
					return false;
				}
			}
			
			if (!acc.close_position())
			{
				ERROR("Backtest: Account: %s", acc.error());
				return false;
			}

			return true;
		}

		bool backtest_interval(PaperAccount* best, const Asset* asset, const Strategy* strat,
			int interval, long long permutations, double principal, double minimum, double fee, bool shorting_enabled,
			int leverage, int min_range, int max_range, int granularity, const std::vector<int>& start_ranges)
		{
			std::vector<int> curr_ranges = start_ranges;
			const Client* client;// = asset->client();
			CandleSet candles = client->get_candles(asset->ticker(), interval, 0, 0);
			
			*best = PaperAccount(principal, leverage, fee, minimum, candles.front().open(),
				shorting_enabled, interval, curr_ranges);

			for (int i = 0; i < permutations; i++)
			{
				// storing activity and performance data
				PaperAccount acc(principal, leverage, fee, minimum, candles.front().open(),
					shorting_enabled, interval, curr_ranges);

				backtest_permutation(acc, candles, strat, curr_ranges);

				if (acc.equity() > best->equity())
				{
					*best = acc;
				}

				// incrementing the permutation

				int pos = 0;
				curr_ranges[pos] += granularity;
				while (curr_ranges[pos] > max_range)
				{
					curr_ranges[pos] = min_range;
					pos++;
					curr_ranges[pos] += granularity;
				}
			}
			return true;
		}

		PaperAccount backtest(const Asset* asset)
		{
			const Client* client;// = asset->client();
			const Strategy& strategy = asset->strategy();

			AccountInfo account_info;// = asset->client()->get_account_info();
			AssetInfo asset_info;// = asset->client()->get_asset_info(asset->ticker());
			CandleSet candles = client->get_candles(asset->ticker(), asset->interval(), 0, 0);
			PaperAccount account
			{
				account_info.balance(),
				account_info.leverage(),
				asset_info.fee(),
				asset_info.minimum(),
				candles.front().open(),
				account_info.shorting_enabled(),
				asset->interval(),
				asset->ranges()
			};
			
			//backtest_permutation(account, candles, strategy, asset->ranges());

			return account;
		}

		std::vector<PaperAccount> backtest(int strat_index, int asset_index, double principal,
			bool shorting_enabled, int min_range, int max_range, int granularity,
			const std::vector<int>& test_ranges)
		{
			auto t0 = std::chrono::system_clock::now();
			const Asset* asset;// = get_asset(asset_index);
			const Client* client;// = asset->client();
			const Strategy* strat;// = get_strategy(strat_index);

			// storing important info
			std::vector<int> intervals = client->backtest_intervals();

			INFO("Backtesting '%s' with strategy '%s'...", asset->ticker(), strat->filename());

			// calculating lops
			std::vector<int> start_ranges;
			long long permutations = 1;
			int possible_vals = ((max_range + 1) - min_range) / granularity;

			// if no ranges are passed in
			if (test_ranges.empty())
			{
				// setting default ranges
				start_ranges.resize(strat->indicator_count(), min_range);

				// calculate the amount of permutations of ranges
				for (int i = 0; i < strat->indicator_count(); i++) permutations *= possible_vals;
			}
			else
			{
				// setting default ranges to the given ones
				start_ranges = test_ranges;

				// verify ranges size
				if (start_ranges.size() >  strat->indicator_count())
				{
					ERROR("Backtest: Passed in %d ranges but %d were expected! Resizing ranges...", start_ranges.size(), strat->indicator_count());
					start_ranges.resize(strat->indicator_count());
				}
				else if (start_ranges.size() < strat->indicator_count())
				{
					ERROR("Backtest: Passed in %d ranges but %d were expected! Execution cannot continue.", start_ranges.size(), strat->indicator_count());
					return {};
				}

				// verify minimums and maximums
				for (int i = 0; i < start_ranges.size(); i++)
				{
					if (start_ranges[i] < min_range)
					{
						start_ranges[i] = min_range;
						WARNING("test_ranges[%d] was less than the minimum (%d). Readjusting...", start_ranges[i], min_range);
					}
					else if (start_ranges[i] > max_range)
					{
						start_ranges[i] = max_range;
						WARNING("test_ranges[%d] was greater than the maximum (%d). Readjusting...", start_ranges[i], max_range);
					}
				}
			}

			AccountInfo acct = client->get_account_info();
			AssetInfo info = client->get_asset_info(asset->ticker());
			std::vector<PaperAccount> out(intervals.size());
			std::vector<std::future<bool>> threads(intervals.size());
			// for every intervals
			for (int i = 0; i < intervals.size(); i++)
			{
				threads[i] = std::async(std::launch::async, backtest_interval, &out[i], asset,
				strat, intervals[i], permutations, principal, info.minimum(), info.fee(), shorting_enabled, acct.leverage(),
				min_range, max_range, granularity, start_ranges);
				
			}

			for (int i = 0; i < threads.size(); i++)
			{
				if (threads[i].get())
				{
					SUCCESS("Backtest %d succeeded", i + 1);
				}
				else
				{
					ERROR("Backtest %d failed", i + 1);
				}
				
			}
			auto t = std::chrono::system_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0);
			double secs = ms.count() / 1000.0;

			SUCCESS("Backtesting of %s has completed in %fs", asset->ticker(), secs);

			return out;
		}

		std::vector<PaperAccount> backtest(Client* client, const Strategy* strat, const std::vector<int>& ranges)
		{
			CandleSet candles = client->get_candles("EUR_USD", 300, 0, 0);
			PaperAccount acc0(500, 1, 0.0001, 1.0, candles.front().open(), false, 300, ranges);
			PaperAccount acc1(500, 10, 0.0001, 1.0, candles.front().open(), false, 300, ranges);

			backtest_permutation(acc0, candles, strat, ranges);
			backtest_permutation(acc1, candles, strat, ranges);

			return { acc0, acc1 };
		}

		/*
		PaperAccount& acc, CandleSet& candles, const Strategy* strat,
			const std::vector<int>& ranges
		*/
	}
}