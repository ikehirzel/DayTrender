#include "shell.h"

#include "interface.h"
#include "../daytrender.h"

#include <vector>
#include <iostream>
#include <unordered_map>

#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

/*
	TODO:
		Implement shorting_enabled in backtest call
*/

namespace daytrender
{
	namespace shell
	{
		void backtest(const std::vector<std::string>& tokens);
		void exit(const std::vector<std::string>& tokens);

		std::unordered_map<std::string, void(*)(const std::vector<std::string>&)> shell_funcs =
		{
			{ "backtest", backtest},
			{ "exit", exit }
		};

		void exit(const std::vector<std::string>& tokens)
		{
			if (tokens.size() > 1)
			{
				ERROR("exit: invalid use of command. exit does not take arguments");
			}
			else
			{
				daytrender::stop();
			}
		}

		void backtest(const std::vector<std::string>& tokens)
		{
			if (tokens.size() == 3)
			{
				int strat_index = -1;
				int asset_index = -1;

				// tokens[1] should be strat name and 2 should be ticker
				// auto strat_filenames = strategy_names();

				// for (int i = 0; i < strat_filenames.size(); i++)
				// {
				// 	if (tokens[1] == strat_filenames[i])
				// 	{
				// 		strat_index = i;
				// 		break;
				// 	}
				// }

				if (strat_index < 0)
				{
					ERROR("backtest: invalid strat name given!");
					return;
				}

				// auto asset_info = asset_names();

				// for (int i = 0; i < asset_info.size(); i++)
				// {
				// 	if (tokens[2] == asset_info[i].first)
				// 	{
				// 		asset_index = i;
				// 		break;
				// 	}
				// }

				if (asset_index < 0)
				{
					ERROR("backtest: invalid ticker given!");
					return;
				}

				auto result = interface::backtest(strat_index, asset_index, 500.0, false, 5, 155, 10, {});

				for (const PaperAccount& p : result)
				{
					std::cout << p << std::endl;
				}

			}
			else
			{
				ERROR("backtest: incorrect usage of command! correct usage is backtest <strategy> <asset-type> <ticker>");
			}
		}

		void get_input()
		{
			std::string input;

			while (daytrender::is_running())
			{
				std::getline(std::cin, input);
				INFO("$ %s", input);

				if(input.empty())
				{
					continue;
				}

				void (*func)(const std::vector<std::string>&) = nullptr;
				std::vector<std::string> tokens = hirzel::str::tokenize(input, " \t");

				func = shell_funcs[tokens[0]];

				if (!func)
				{
					ERROR("shell: invalid command!");
				}
				else
				{
					func(tokens);
				}
			}
		}
	}
}