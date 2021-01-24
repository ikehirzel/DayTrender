#include "interface.h"

#include "../daytrender.h"
#include "../api/action.h"
#include <hirzel/fountain.h>

namespace daytrender
{
	namespace interface
	{
		std::vector<PaperAccount> backtest(int algo_index, int asset_index, const std::vector<int>& test_ranges)
		{
			const Asset* asset = get_asset(asset_index);
			Client* client = asset->client();
			const Algorithm* algo = get_algorithm(algo_index);

			std::string ticker = asset->ticker();
			double paper_initial = PAPER_ACCOUNT_INITIAL;
			double risk = asset->risk();
			double paper_fee = client->paper_fee();
			double paper_minimum = client->paper_minimum();
			std::vector<int> intervals = client->backtest_intervals();

			std::vector<PaperAccount> out(intervals.size());

			std::vector<CandleSet> candles_vec(intervals.size());
			for (int i = 0; i < candles_vec.size(); i++)
			{
				candles_vec[i] = client->get_candles(ticker, intervals[i]);

				if (candles_vec[i].empty())
				{
					errorf("Cannot continue backtest as not all candles were received!");
					return {};
				}
			}

			infof("Backtesting algorithm...");

			// calculating lops
			std::vector<int> ranges, start_ranges;
			#define SKIP_AMT 5
			long long permutations = 1;
			int possible_vals = ((MAX_ALGORITHM_WINDOW + 1) - MIN_ALGORITHM_WINDOW) / SKIP_AMT;
			// if no ranges are passed in
			if (test_ranges.empty())
			{
				// setting default ranges
				start_ranges.resize(algo->get_ranges_count(), MIN_ALGORITHM_WINDOW);

				// calculate the amount of permutations of ranges
				for (int i = 0; i < algo->get_ranges_count(); i++) permutations *= possible_vals;
			}
			else
			{
				// setting default ranges to the given ones
				start_ranges = test_ranges;

				// verify ranges size
				if (start_ranges.size() !=  algo->get_ranges_count())
				{
					errorf("Passed in %d ranges to backtest but %d were expected! resizing ranges...", start_ranges.size(), algo->get_ranges_count());
					start_ranges.resize(algo->get_ranges_count());
				}

				// verify minimums and maximums
				for (int i = 0; i < ranges.size(); i++)
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

			// for every intervals
			for (int i = 0; i < intervals.size(); i++)
			{
				PaperAccount best;
				double bahnr = 0.0;
				// re-init with minimum range for each arg
				ranges = start_ranges;

				// for every possible permutation of ranges for indicators/algorithm
				for (int j = 0; j < permutations; j++)
				{
					// storing activity and performance data
					PaperAccount acc(PAPER_ACCOUNT_INITIAL, paper_fee, paper_minimum, intervals[i], candles_vec[i][0].open, ranges);

					// calculating size of candles
					int candle_count = 0;
					int longest_indi = 0;
					if (ranges.size() > 1)
					{
						for (int v = 1; v < ranges.size(); v++)
						{
							if (ranges[v] > longest_indi) longest_indi = ranges[v];
						}
					}
					candle_count = ranges[0] + longest_indi;
					//std::cout << "candle count: " << candle_count << std::endl;

					// walk through every step of candles_vec[i]
					for (int k = 0; k < candles_vec[i].size() - candle_count; k++)
					{
						CandleSet candles = candles_vec[i].slice(k, candle_count);
						acc.update_price(candles.back().close);
						AlgorithmData data = algo->process(candles, ranges);

						if (data.error())
						{
							errorf("Backtest: %s", data.error());
						}
						else
						{
							action::paper_actions[data.action()](acc, risk);
						}
					}

					// comparing account to previous, potentially storing account

					double ahnr = acc.avg_net_per_hour();
					
					if (ahnr > bahnr)
					{
						bahnr = ahnr;
						best = acc;
					}

					// incrementing the permutation

					int pos = 0;
					ranges[pos] += SKIP_AMT;
					while (ranges[pos] > MAX_ALGORITHM_WINDOW)
					{
						ranges[pos] = MIN_ALGORITHM_WINDOW;
						pos++;
						ranges[pos] += SKIP_AMT;
					}
				}

				std::cout << "Completed backtest " << i << '\n';
				out[i] = best;
			}

			return out;
		}
	}
}