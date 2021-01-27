#include "interface.h"

#include "../daytrender.h"
#include "../api/action.h"
#include <hirzel/fountain.h>

namespace daytrender
{
	namespace interface
	{
		std::vector<PaperAccount> backtest(int algo_index, int asset_index, int granularity, const std::vector<int>& test_ranges)
		{
			const Asset* asset = get_asset(asset_index);
			Client* client = asset->client();
			const Algorithm* algo = get_algorithm(algo_index);

			// storing important info
			std::vector<int> intervals = client->backtest_intervals();

			std::vector<CandleSet> candles_vec(intervals.size());
			for (int i = 0; i < candles_vec.size(); i++)
			{
				candles_vec[i] = client->get_candles(asset->ticker(), intervals[i]);

				if (candles_vec[i].empty())
				{
					errorf("Cannot continue backtest as not all candles were received!");
					return {};
				}
			}

			infof("Backtesting %s asset '%s' with algorithm '%s'...", client->label(), asset->ticker(), algo->filename());

			// calculating lops
			std::vector<int> curr_ranges, start_ranges;
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
				for (int i = 0; i < curr_ranges.size(); i++)
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

			std::vector<PaperAccount> out(intervals.size(),
			{
				PAPER_ACCOUNT_INITIAL,
				client->leverage(),
				client->paper_fee(),
				client->paper_minimum(),
				candles_vec[0][0].open(),
				intervals[0],
				curr_ranges
			});

			// for every intervals
			for (int i = 0; i < intervals.size(); i++)
			{
				// re-init with minimum range for each arg
				curr_ranges = start_ranges;

				// for every possible permutation of ranges for indicators/algorithm
				for (int j = 0; j < permutations; j++)
				{
					// storing activity and performance data
					PaperAccount acc(PAPER_ACCOUNT_INITIAL, client->leverage(), client->paper_fee(), client->paper_minimum(), candles_vec[i][0].open(), intervals[i], curr_ranges);
					
					// calculating size of candles
					int candle_count = 0;
					if (curr_ranges.size() > 1)
					{
						for (int v = 1; v < curr_ranges.size(); v++)
						{
							if (curr_ranges[v] > candle_count) candle_count = curr_ranges[v];
						}
					}
					candle_count += curr_ranges[0];
					
					// walk through every step of candles_vec[i]
					for (int k = 0; k < candles_vec[i].size() - candle_count; k++)
					{
						CandleSet candles = candles_vec[i].slice(k, candle_count, 0);
						acc.update_price(candles.back().close());
						AlgorithmData data = algo->process(candles, curr_ranges);

						if (data.error())
						{
							errorf("Backtest: %s", data.error());
							return {};
						}

						if (!action::paper_actions[data.action()](acc, asset->risk()))
						{
							errorf("Backtest: PaperAccount: %s", acc.error());
							return {};
						}
					}

					if (acc.net_per_year() > out[i].net_per_year() || j == 0)
					{
						out[i] = acc;
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

				std::cout << "Completed backtest " << i << '\n';
			}

			return out;
		}
	}
}