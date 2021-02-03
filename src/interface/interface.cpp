#include "interface.h"

#include "../daytrender.h"

#include <hirzel/fountain.h>

#include <future>
#include <chrono>

namespace daytrender
{
	namespace interface
	{
		//bool backtest_permutation(

		bool backtest_interval(PaperAccount* best, const Asset* asset, const Algorithm* algo,
			int interval, long long permutations, int granularity, const std::vector<int>& start_ranges)
		{
			std::vector<int> curr_ranges = start_ranges;
			const Client* client = asset->client();
			CandleSet candles = client->get_candles(asset->ticker(), interval, 0, 0);
			
			for (int i = 0; i < permutations; i++)
			{
				/*
				 	TODO:
					 	Implement shorting enabled
				*/

				// storing activity and performance data
				PaperAccount acc(PAPER_ACCOUNT_INITIAL, client->leverage(), client->fee(),
					client->order_minimum(), candles.front().open(), false, interval, curr_ranges);
				
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


				for (long j = 0; j < candles.size() - candle_count; j++)
				{
					CandleSet slice = candles.slice(j, candle_count, algo->data_length());
					if (slice.empty())
					{
						std::cout << "Candles size: " << candles.size() << std::endl;
						std::cout << "size: " << slice.size() <<  std::endl;
						std::cout << "end: " << slice.end() <<  std::endl;
					}

					if (slice.error())
					{
						errorf("Backtest: CandleSet: %s", slice.error());
						return false;
					}
					
					acc.update_price(slice.back().close());
					AlgorithmData data = algo->process(slice, curr_ranges);

					if (data.error())
					{
						errorf("Backtest: Algorithm: %s", data.error());
						return false;
					}

					if (!acc.handle_action(data.action()))
					{
						errorf("Backtest: Account: %s", acc.error());
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
			auto t0 = std::chrono::system_clock::now();
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
				start_ranges.resize(algo->indicator_count(), MIN_ALGORITHM_WINDOW);

				// calculate the amount of permutations of ranges
				for (int i = 0; i < algo->indicator_count(); i++) permutations *= possible_vals;
			}
			else
			{
				// setting default ranges to the given ones
				start_ranges = test_ranges;

				// verify ranges size
				if (start_ranges.size() >  algo->indicator_count())
				{
					errorf("Backtest: Passed in %d ranges but %d were expected! Resizing ranges...", start_ranges.size(), algo->indicator_count());
					start_ranges.resize(algo->indicator_count());
				}
				else if (start_ranges.size() < algo->indicator_count())
				{
					errorf("Backtest: Passed in %d ranges but %d were expected! Execution cannot continue.", start_ranges.size(), algo->indicator_count());
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
			auto t = std::chrono::system_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0);
			double secs = ms.count() / 1000.0;

			successf("Backtesting of %s has completed in %fs", asset->ticker(), secs);

			return out;
		}
	}
}