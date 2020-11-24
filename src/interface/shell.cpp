#include "shell.h"

#include "../daytrender.h"
#include <vector>

#include <hirzel/fountain.h>
#include <hirzel/strutil.h>
#include <iostream>

namespace daytrender
{
	void shell_getInput()
	{
		std::string input;

		while (isRunning())
		{
			std::getline(std::cin, input);
			infof("$ %s", input);
			if(input.empty())
			{
				continue;
			}

			std::vector<std::string> tokens = hirzel::tokenize(input, " \t");
			/************************************************************
			 * Exit
			 ***********************************************************/
			if(tokens[0] == "exit")
			{
				stop();
				break;
				// call daytrender stop();
			}
			/************************************************************
			 *	Backtest
			 ************************************************************/
			else if (tokens[0] == "backtest")
			{
				bool found = false;
				
				if (tokens.size() == 4)
				{
					auto result = backtest(tokens[1], tokens[2], tokens[3]);
					for (const PaperAccount& p : result)
					{
						std::cout << p << std::endl;
					}

				}
				else
				{
					errorf("Incorrect usage of command: correct usage is backtest <algorithm> <asset-type> <ticker>");
				}
			}
			/************************************************************
			 *	Build
			 ************************************************************/
			else if (tokens[0] == "build")
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
			/************************************************************
			 *	Default
			 ************************************************************/
			else
			{
				warningf("dtshell: Command not found");
			}
		}
	}
}