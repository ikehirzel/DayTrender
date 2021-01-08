#include "shell.h"

#include "../daytrender.h"

#include <vector>
#include <iostream>
#include <unordered_map>

#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

namespace daytrender
{
	namespace shell
	{
		std::unordered_map<std::string, void(*)(const std::vector<std::string>&)> shell_funcs;

		void exit(const std::vector<std::string>& tokens)
		{
			if (tokens.size() > 1)
			{
				errorf("exit: invalid use of command. exit does not take arguments");
			}
			else
			{
				daytrender::stop();
			}
		}

		void build(const std::vector<std::string>& tokens)
		{
			bool build = true;
			std::string filename;
			bool print = false;
			
			for(unsigned int i = 1; i < tokens.size(); i++)
			{
				infof("Tok[%d]: %s", i, tokens[i]);
				if(tokens[i][0] == '-')
				{
					if(tokens[i].size() == 1)
					{
						warningf("'-' is used to denote options!");
					}
					for (unsigned c = 1; c < tokens[i].size(); c++)
					{
						switch (tokens[i][c])
						{
							case 'p':
							case 'P':
								print = true;
								break;

							default:
								warningf("Unknown option '%c' used in command", tokens[i][c]);
								break;
						}
					}
				}
				else
				{
					if (filename.empty())
					{
						filename = tokens[i];
					}
					else
					{
						errorf("Only one input file should be specified! Aborting...");
						build = false;
					}
				}
			}

			if (build)
			{
				build = buildAlgorithm(filename, print);
				if (build)
				{
					successf("Successfully compiled %s", tokens[1]);
				}
				else
				{
					errorf("Failed to compile %s", tokens[1]);
				}
			}
		}

		void backtest(const std::vector<std::string>& tokens)
		{
			if (tokens.size() == 3)
			{
				int algo_index = -1;
				int asset_index = -1;

				// tokens[1] should be algo name and 2 should be ticker
				auto algo_filenames = getAlgoInfo();

				for (int i = 0; i < algo_filenames.size(); i++)
				{
					if (tokens[1] == algo_filenames[i])
					{
						algo_index = i;
						break;
					}
				}

				if (algo_index < 0)
				{
					errorf("backtest: invalid algorithm name given!");
					return;
				}

				auto asset_info = getAssetInfo();

				for (int i = 0; i < asset_info.size(); i++)
				{
					if (tokens[2] == asset_info[i].first)
					{
						asset_index = i;
						break;
					}
				}

				if (asset_index < 0)
				{
					errorf("backtest: invalid ticker given!");
					return;
				}

				auto result = daytrender::backtest(algo_index, asset_index);

				for (const PaperAccount& p : result)
				{
					std::cout << p << std::endl;
				}

			}
			else
			{
				errorf("backtest: incorrect usage of command! correct usage is backtest <algorithm> <asset-type> <ticker>");
			}
		}

		void init()
		{
			shell_funcs["backtest"] = backtest;
			shell_funcs["build"] = build;
			shell_funcs["exit"] = exit;
		}

		void get_input()
		{
			std::string input;

			while (daytrender::isRunning())
			{
				std::getline(std::cin, input);
				infof("$ %s", input);

				if(input.empty())
				{
					continue;
				}

				void (*func)(const std::vector<std::string>&) = nullptr;
				std::vector<std::string> tokens = hirzel::str::tokenize(input, " \t");

				func = shell_funcs[tokens[0]];

				if (!func)
				{
					errorf("shell: invalid command!");
				}
				else
				{
					func(tokens);
				}
			}
		}
	}
}