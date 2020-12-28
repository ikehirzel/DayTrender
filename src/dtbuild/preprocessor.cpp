#include "preprocessor.h"

#include <vector>
#include <iostream>

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>

namespace dtbuild
{

	size_t read_directive(std::vector<std::string>& args, const std::string& src, size_t i)
	{
		bool reading = false;
		bool in_string = false;
		if (!args.empty()) args.clear();
		size_t oi = i;
		i++;
		while (i < src.size())
		{
			if (src[i] == '\n')
			{
				break;
			}
			else if (src[i] == '\"')
			{
				in_string = !in_string;
			}

			if (reading)
			{
				if (src[i] < 33)
				{
					reading = false;
				}
				else
				{
					args.back() += src[i];
				}
			}
			else
			{
				if (src[i] > 32)
				{
					reading = true;
					args.push_back(std::string(1, src[i]));
				}
			}

			i++;
		}
		return (i - oi) + 1;
	}

	std::string read_and_preprocess(const std::string& filepath)
	{		
		std::string src = hirzel::file::read_file_as_string(filepath);

		if (src.empty()) return " ";

		src.insert(0, "@1:" + filepath + '\n');

		// Stripping comments
		size_t fi = 0;
		for (size_t i = 0; i < src.size(); i++)
		{
			src[fi] = src[i];
			// testing for comment
			
			if (src[i] == '/')
			{
				i++;

				if (i == src.size())
				{
					fi++;
					break;
				}

				// line comment
				if (src[i] == '/')
				{
					i++;
					while (i < src.size())
					{
						if (src[i] == '\n')
						{
							i--;
							break;
						}
						i++;
					}
					src[fi] = ' ';
				}
				// start of block comment
				else if (src[i] == '*')
				{
					i += 2;
					while (i < src.size())
					{
						if (src[i - 1] == '*' && src[i] == '/') break;
						i++;
					}
					src[fi] = ' ';
				}
				else
				{
					fi++;
				}
			}
			fi++;
		}

		src.resize(fi);

		// Handling preprocessor directives
		int state = 0;
		std::string cmd, tmp, presrc;
		std::vector<std::string> args;

#define LOOKING_FOR_PRE		0
#define LOOKING_FOR_CMD 	1
#define READING_CMD			2
#define LOOKING_FOR_ARGS	3
#define READING_ARGS		4

		bool in_string = false;
		bool escape = false;
		fi = 0;

		// this is to make sure that src[i - 1] will never be out of bounds
		size_t tmpi = 0;
		bool newline = true;
		long line = 1;
		for (size_t i = 0; i < src.size(); i++)
		{
			if (src[i] == '\n')
			{
				line++;
				newline = true;
			}
			else if (newline)
			{
				if (src[i] > 32) newline = false;
				if (src[i] == '#')
				{
					size_t offs = read_directive(args, src, i);

					if (args.empty())
					{
						std::cout << "Stray '#' found in program!\n";
						return "";
					}
					else if (args.size() < 2)
					{
						std::cout << "No arguments given to preprocessor directive #" << args[0] << '\n';
						return "";
					}

					if (args[0] == "require")
					{
						std::string glob;
						for (int i = 1; i < args.size(); i++)
						{
							args[i] = hirzel::str::get_folder(filepath) + "/" + args[i].substr(1, args[i].size() - 2);
							std::string inc = read_and_preprocess(args[i]);
							glob += inc;
						}
						glob += '@' + std::to_string(line) + ':' + filepath + '\n';
						std::string pre = src.substr(0, i);
						std::string post = src.substr(i + offs);
						src = pre + glob + post;
						i = pre.size() + glob.size() - 2;
					}
					else
					{
						std::cout << "Unknown preprocessor directive #" << args[0] << '\n';
						return "";
					}
				};
			}
		}

		return src;
	}
}