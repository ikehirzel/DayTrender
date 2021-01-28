#include "interface.h"

#include "../daytrender.h"
#include "../api/action.h"

#include <hirzel/fountain.h>

#include <future>
#include <mutex>

namespace daytrender
{
	namespace interface
	{
		std::mutex mtx;

		bool backtest_interval(PaperAccount* best, const Asset* asset, const Algorithm* algo,
			int interval, long long permutations, int granularity, const std::vector<int>& start_ranges)
		{
			std::vector<int> curr_ranges = start_ranges;
			const Client* client = asset->client();
			CandleSet candles = client->get_candles(asset->ticker(), interval);
			
			for (int i = 0; i < permutations; i++)
			{
				// storing activity and performance data
				PaperAccount acc(PAPER_ACCOUNT_INITIAL, client->leverage(), client->paper_fee(),
					client->paper_minimum(), candles[0].open(), interval, curr_ranges);
				
				// calculating size of candles
				int candle_count = 0;
				if (curr_ranges.size() > 1)
				{
					for (int j = 1; j < curr_ranges.size(); j++)
					{
						if (curr_ranges[j] > candle_count) candle_count = curr_ranges[j];
					}
				}
				candle_count += curr_ranges[0];


				for (int j = 0; j < candles.size() - candle_count; j++)
				{
					CandleSet slice = candles.slice(j, candle_count, 0);
					if (candles.error())
					{
						errorf("Backtest: %s", candles.error());
						return false;
					}
					
					acc.update_price(slice.back().close());
					AlgorithmData data = algo->process(slice, curr_ranges);

					if (data.error())
					{
						errorf("Backtest: %s", data.error());
						return false;
					}

					if (!action::paper_actions[data.action()](acc, asset->risk()))
					{
						errorf("Backtest: %s", acc.error());
						return false;
					}
				}

				if (acc.net_per_year() > best->net_per_year() || i == 0)
				{
					*best = acc;
				}

				// incrementing the permutation

				int pos = 0;
				curr_ranges[pos] += granularity;
				while (curr_ranges[pos] > MAX_ALGORITHM_WINDOW)
				{
					curr_ranges[pos] = MIN_ALGORITHM_WINDOW;
					pos++;
					curr_ranges[pos] += granularity;
				}
			}
			return true;
		}

		std::vector<PaperAccount> backtest(int algo_index, int asset_index, int granularity,
			const std::vector<int>& test_ranges)
		{
			const Asset* asset = get_asset(asset_index);
			const Client* client = asset->client();
			const Algorithm* algo = get_algorithm(algo_index);

			// storing important info
			std::vector<int> intervals = client->backtest_intervals();

			infof("Backtesting %s asset '%s' with algorithm '%s'...", client->label(), asset->ticker(), algo->filename());

			// calculating lops
			std::vector<int> start_ranges;
			long long permutations = 1;
			int possible_vals = ((MAX_ALGORITHM_WINDOW + 1) - MIN_ALGORITHM_WINDOW) / granularity;

			// if no ranges are passed in
			if (test_ranges.empty())
			{
				// setting default ranges
				start_ranges.resize(algo->ranges_count(), MIN_ALGORITHM_WINDOW);

				// calculate the amount of permutations of ranges
				for (int i = 0; i < algo->ranges_count(); i++) permutations *= possible_vals;
			}
			else
			{
				// setting default ranges to the given ones
				start_ranges = test_ranges;

				// verify ranges size
				if (start_ranges.size() >  algo->ranges_count())
				{
					errorf("Backtest: Passed in %d ranges but %d were expected! Resizing ranges...", start_ranges.size(), algo->ranges_count());
					start_ranges.resize(algo->ranges_count());
				}
				else if (start_ranges.size() < algo->ranges_count())
				{
					errorf("Backtest: Passed in %d ranges but %d were expected! Execution cannot continue.", start_ranges.size(), algo->ranges_count());
					return {};
				}

				// verify minimums and maximums
				for (int i = 0; i < start_ranges.size(); i++)
				{
					if (start_ranges[i] < MIN_ALGORITHM_WINDOW)
					{
						start_ranges[i] = MIN_ALGORITHM_WINDOW;
						warningf("test_ranges[%d] was less than the minimum (%d). Readjusting...", start_ranges[i], MIN_ALGORITHM_WINDOW);
					}
					else if (start_ranges[i] > MAX_ALGORITHM_WINDOW)
					{
						start_ranges[i] = MAX_ALGORITHM_WINDOW;
						warningf("test_ranges[%d] was greater than the maximum (%d). Readjusting...", start_ranges[i], MIN_ALGORITHM_WINDOW);
					}
				}
			}

			std::vector<PaperAccount> out(intervals.size());
			std::vector<std::future<bool>> threads(intervals.size());
			// for every intervals
			for (int i = 0; i < intervals.size(); i++)
			{
				threads[i] = std::async(std::launch::async, backtest_interval, &out[i], asset, algo, intervals[i], permutations, granularity, start_ranges);
			}

			for (int i = 0; i < threads.size(); i++)
			{
				if (threads[i].get())
				{
					successf("Backtest %d succeeded", i);
				}
				else
				{
					errorf("Backtest %d failed", i);
				}
			}

			successf("Backtesting of %s has completed", asset->ticker());

			return out;
		}
	}
}