#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cctype>
#include <filesystem>
#include <unordered_map>

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include "../data/action.h"

#define TEMP_OUTPUT_EXTENSION ".cpp"

#if defined(_WIN32) || defined(_WIN64)
#define OUTPUT_EXTENSION ".dll"
#elif defined(linux) || defined(__unix__)
#define OUTPUT_EXTENSION ".so"
#endif

#define CONFIG_FILENAME "./dtbuild.config"
#define ALGODEF_FILENAME "algorithm.def"
#define INDIDEF_FILENAME "indicator.def"
#define INDICATOR_FOOTER "_INDICATOR"

namespace daytrender
{
}

// hooks for replacement

// These occur in algorithm.def
#define ALGORITHM_NAME_HOOK "%ALGORITHM_NAME%"
#define ALGORITHM_SCRIPT_HOOK "%ALGORITHM_SCRIPT%"
#define INDICATOR_DEFINE_HOOK "%INDICATOR_DEFINE%"
#define INDICATOR_DECLARE_HOOK "%INDICATOR_DECLARE%"
#define DATASET_INIT_HOOK "%DATASET_INIT%"
#define INDICATOR_DATA_HOOK "%INDICATOR_DATA%"
#define INCLUDES_HOOK "%INCLUDES%"

// These occur in indicator.def
#define INDICATOR_CLASS_HOOK "%INDICATOR_CLASS%"
#define INDICATOR_LABEL_HOOK "%INDICATOR_LABEL%"
#define CALCULATE_SCRIPT_HOOK "%CALCULATE_SCRIPT%"

#define LABEL_TOKEN "$label"
#define IDENTIFIER_TOKEN "$identifier"
#define REQUIRE_TOKEN "#require"
#define INCLUDE_TOKEN "#include"
#define AS_TOKEN "as"

#define ACTION_BUY_PROXY "buy();"
#define ACTION_SELL_PROXY "sell();"
#define ACTION_NOTHING_PROXY "do_nothing();"

#include <filesystem>

using namespace hirzel;

bool failed = false;

//recursively find the files required
std::vector<std::string> get_requires(const std::string& dir, const std::string &filename, std::unordered_map<std::string, bool>& reqd)
{
	if(reqd[filename] == true)
	{
		failed = true;
		std::cout << "dtbuild: error: circular dependency detected in '" << filename << "'!\n";
		return {};
	}
	reqd[filename] = true;

	std::vector<std::string> file = read_file(dir + "/indicators/" + filename);
	std::vector<std::string> reqs;
	reqs.insert(reqs.begin(), filename);
	for (std::string line : file)
	{
		std::vector<std::string> toks = hirzel::tokenize(line, "\t ");
		if (toks[0] == REQUIRE_TOKEN)
		{
			std::string subname = get_filename(purge_delims(toks[1], "\""));
			std::vector<std::string> subreqs = get_requires(dir, subname, reqd);
			for (int i = subreqs.size() - 1; i >= 0; i--)
			{
				reqs.insert(reqs.begin(), subreqs[i]);
			}
		}
	}

	return reqs;
}

//takes in path to indicators script and returns a pair of its identifier and definition
std::pair<std::string, std::string> compose_indicator(const std::string& dir, const std::string &filename)
{
	std::string buffer
	{
		#include "indicator.def"
	};
	std::string label, script, classname, includes;
	std::vector<std::string> file = hirzel::read_file(dir + "/indicators/" + filename);

	if (file.empty())
	{
		std::cout << "dtbuild: error: Failed to load file '" + filename + "'\n";
		failed = true;
		return {"", ""};
	}

	script += "\n";

	unsigned int lineIndex = 0;
	for (std::string s : file)
	{
		std::vector<std::string> toks = hirzel::tokenize(s, "\t ");
		if (toks[0][0] == '$' || toks[0][0] == '#')
		{
			if (toks[0] == LABEL_TOKEN)
			{
				if (!label.empty())
				{
					std::cout << "dtbuild: error: Redefinition of indicator label!\n";
					failed = true;
					return { "", "" };
				}
				int startIndex = -1, endIndex = -1;
				for (unsigned int i = 0; i < s.size(); i++)
				{
					if (s[i] == '"')
					{
						if (startIndex == -1)
						{
							startIndex = i + 1;
						}
						else
						{
							endIndex = i - 1;
							break;
						}
					}
				}

				label = s.substr(startIndex, endIndex - startIndex + 1);

			}
			else if (toks[0] == IDENTIFIER_TOKEN)
			{
				classname = toks[1];
			}
			else if (toks[0] == REQUIRE_TOKEN)
			{
				// TODO : allow for including other indicators
				break;
			}
			else if (toks[0] == INCLUDE_TOKEN)
			{
				// TODO : allow for including normal c++ files
				includes += s + "\n";
			}
			else
			{
				std::cout << "dtbuild: error: Syntax error: line " << lineIndex << std::endl;
				failed = true;
				return {};
			}
			lineIndex++;
			continue;
		}

		script += "\n\t\t" + s;
		lineIndex++;
	}
	if (label.empty())
	{
		std::cout << "dtbuild: error: $label must be defined in indicator '" + filename + "'\n";
		failed = true;
		return {};
	}
	replace_hook(buffer, INCLUDES_HOOK, includes);
	replace_hook(buffer, INDICATOR_CLASS_HOOK, classname);
	replace_hook(buffer, INDICATOR_LABEL_HOOK, label);
	replace_hook(buffer, CALCULATE_SCRIPT_HOOK, script);
	return { classname, buffer };
}

/*
	main calls compose algorithm
	compose algorithm scans for requires,
	compose algorithm calls compose indicator for each scanned require

	TODO:
		---Add include guards to required files so that no algorithm or indicator double
		requires a file. This may require passing required_filenames into compose_indicator
		so that each indicator knows as well considering they will be able to require
		as well.
*/

std::string compose_algorithm(const std::string &dir, const std::string &filepath)
{
	std::string buffer
	{
		#include "algorithm.def"
	};
	std::string algo_name, script, indicator_definition_glob, includes;
	std::vector<std::string> input_file;
	std::vector<std::vector<std::string>> indicator_variables;
	std::unordered_map<std::string, std::pair<std::string, std::string>> indicator_definitions;

	// loading script
	input_file = hirzel::read_file(filepath);
	if (input_file.empty())
	{
		std::cout << "dtbuild: fatal: Failed to open input file!\n";
		failed = true;
		return "";
	}

	unsigned int lineIndex = 0;
	for (std::string &s : input_file)
	{
		std::vector<std::string> tokens = hirzel::tokenize(s, " \t");

		if (tokens[0][0] == '$' || tokens[0][0] == '#')
		{
			if (tokens[0] == LABEL_TOKEN)
			{
				if (!algo_name.empty())
				{
					std::cout << "dtbuild: error: Redefinition of algorithm label!\n";
					failed = true;
					return "";
				}
				int startIndex = -1, endIndex = -1;
				for (unsigned int i = 0; i < s.size(); i++)
				{
					if (s[i] == '"')
					{
						if (startIndex == -1)
						{
							startIndex = i + 1;
						}
						else
						{
							endIndex = i - 1;
							break;
						}
					}
				}

				algo_name = s.substr(startIndex, endIndex - startIndex + 1);
			}
			else if (tokens[0] == REQUIRE_TOKEN)
			{
				std::unordered_map<std::string, bool> reqd;
				std::vector<std::string> reqs = get_requires(dir, get_filename(purge_delims(tokens[1], "\"")), reqd);
				for (int i = reqs.size() - 1; i >= 0; i--)
				{
					// checking if the indicator is already defined
					if (indicator_definitions[reqs[i]].first.empty())
					{
						indicator_definitions[reqs[i]] = compose_indicator(dir, reqs[i]);
					}
				}
			}
			else if (tokens[0] == INCLUDE_TOKEN)
			{
				// TODO : allow for including normal c++ files
				includes += s + "\n";
			}
			else
			{
				std::cout << "dtbuild: error: invalid preprocessor directive at line " << lineIndex << "!\n";
				failed = true;
			}

			s = "";
		}
		else if (tokens[0] == ACTION_NOTHING_PROXY)
			{
				s = "\treturn " + std::to_string(ACTION_NOTHING) + ";";
		}
		else if (tokens[0] == ACTION_BUY_PROXY)
		{
			s = "\treturn " + std::to_string(ACTION_BUY) + ";";
		}
		else if (tokens[0] == ACTION_SELL_PROXY)
		{
			s = "\treturn " + std::to_string(ACTION_SELL) + ";";
		}
		else
		{
			for (std::pair<std::string, std::pair<std::string, std::string>> pair : indicator_definitions)
			{
				//if the identifier used matches a declared indicator
				if (tokens[0] == pair.second.first)
				{
					std::vector<std::string> vartoks = hirzel::tokenize(s, " \t();");
					if (vartoks.size() != 3)
					{
						std::cout << "dtbuild: error: Syntax error when declaring indicator: lines " << lineIndex << "!\n";
						failed = true;
						return "";
					}

					indicator_variables.push_back(vartoks);
					s = "";
				}
			}
		}

		lineIndex++;
	} // end of algo parsing for loop

	if (algo_name.empty())
	{
		std::cout << "dtbuild: fatal: A name must be defined!\n";
		failed = true;
		return "";
	}
	// composing each indicator required in file
	for (std::pair<std::string, std::pair<std::string, std::string>> pair : indicator_definitions)
	{
		indicator_definition_glob += pair.second.second + "\n";
	}

	// adding to script string, mainly for human readability
	for (unsigned int i = 0; i < input_file.size(); i++)
	{
		if(!input_file[i].empty())
		{
			script += "\n\t" + input_file[i];
		}
	}

	std::string indi_decl, indi_data, dataset_init;
	for (std::vector<std::string> v : indicator_variables)
	{
		std::string data_var_name = v[1];
		std::string var_name = v[1] + INDICATOR_FOOTER;
		indi_decl += v[0] + " " + var_name + "(" + v[2] + ");\n";
		indi_data += "\n\tstd::vector<double> " + data_var_name + " = dataset.at(\"" + data_var_name + "\").second;";
		dataset_init += "\n\tdataset[\"" + data_var_name + "\"] = " + var_name + ".calculate(candles, index, window);";
	}
	// replacing hooks in output buffer
	replace_hook(buffer, ALGORITHM_NAME_HOOK, algo_name);
	replace_hook(buffer, INCLUDES_HOOK, includes);
	replace_hook(buffer, INDICATOR_DEFINE_HOOK, indicator_definition_glob);
	replace_hook(buffer, INDICATOR_DECLARE_HOOK, indi_decl);
	replace_hook(buffer, INDICATOR_DATA_HOOK, indi_data);
	replace_hook(buffer, ALGORITHM_SCRIPT_HOOK, script);
	replace_hook(buffer, DATASET_INIT_HOOK, dataset_init);

	return buffer;
}

int main(int argc, char *argv[])
{
	std::cout << "PATH: " << std::filesystem::current_path().string() + "/" +std::string(argv[0]) << std::endl;
	if (argc == 1)
	{
		std::cout << "dtbuid: fatal: No input files!\n";
		return 1;
	}

	std::string odir, candle_h, candle_cpp;
	std::string cxx, cxxflags, lflags, srcdir;
	std::string filepath;

	bool setodir = false;
	bool printcode = false;
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "-o")
		{
			setodir = true;
			continue;
		}
		else if (arg == "-p")
		{
			printcode = true;
			continue;
		}

		if (setodir)
		{
			odir = argv[i];
			setodir = false;
		}
		else
		{
			if(filepath.empty())
			{
				filepath = argv[i];
			}
			else
			{
				std::cout << "dtbuild: fatal: Only one input file should be specified!\n";
				return 1;
			}
		}
	}

	std::vector<std::string> config_file = hirzel::read_file(CONFIG_FILENAME);

	if (config_file.empty())
	{
		std::cout << "dtbuild: fatal: Failed to open config file!\n";
		return 1;
	}

	// loading details from config file
	for (std::string s : config_file)
	{
		size_t epos;
		std::string val;
		std::vector<std::string> tokens = hirzel::tokenize(s, " \t=");
		if (tokens[0] == "CXX")
		{
			cxx = tokens[1];
		}
		else if (tokens[0] == "CXXFLAGS")
		{
			for (unsigned int i = 1; i < tokens.size(); i++)
			{
				cxxflags += tokens[i] + " ";
			}
		}
		else if (tokens[0] == "LFLAGS")
		{
			for (unsigned int i = 1; i < tokens.size(); i++)
			{
				lflags += tokens[i] + " ";
			}
		}
		else if (tokens[0] == "DTSRCDIR")
		{
			srcdir = tokens[1];
		}
	}

	if (srcdir.empty())
	{
		std::cout << "dtbuild: fatal: Must supply path of DayTrender src folder in config!\n";
		return 1;
	}

	if (cxx.empty())
	{
		std::cout << "dtbuild: fatal: Must supply a c++ compiler in config!\n";
		return 1;
	}

	// std::cout << "SCRIPT FILENAME: " << script_filepath << std::endl;
	std::string cwd = hirzel::get_folder(filepath);
	// adding extension on end of file

	std::string basename = hirzel::get_basename(filepath);
	std::string temp_output_filename = cwd + "/" + basename + TEMP_OUTPUT_EXTENSION;
	std::string algo_buf = compose_algorithm(cwd, filepath);

	if (failed)
	{
		return 1;
	}

	if (printcode)
	{
		std::cout << algo_buf << std::endl;
	}

	// writing temp file
	hirzel::write_file(temp_output_filename, algo_buf);
	std::string output_name;
	if (odir.empty())
	{
		output_name = cwd;
	}
	output_name += odir + "/" + basename + ".so";
	std::string cmd = cxx + " " + cxxflags + temp_output_filename + " -o " + output_name + " " + lflags;
	// std::cout << "CWD: " << cwd << std::endl;
	if (system(cmd.c_str()))
	{
		failed = true;
		std::cout << "Failed to compile " + output_name + "!\n";
	}
	if (false)//std::remove(temp_output_filename.c_str()))
	{
		failed = true;
		std::cout << "dtbuild: error: Failed to remove temp files!\n";
	}
	if(failed)
	{
		std::cout << "dtbuild: compilation has failed!\n";
	}

	return failed;
}
