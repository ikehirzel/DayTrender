#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cctype>
#include <filesystem>
#include <unordered_map>
#include <map>
#include <stdio.h>
#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include "../data/action.h"

#define TEMP_OUTPUT_EXTENSION ".cpp"

#if defined(_WIN32) || defined(_WIN64)
#define OUTPUT_EXTENSION ".dll"
#elif defined(linux) || defined(__unix__)
#define OUTPUT_EXTENSION ".so"
#endif

#define CONFIG_FILENAME "config/dtbuild.config"
#define ALGODEF_FILENAME "algorithm.def"
#define INDIDEF_FILENAME "indicator.def"
#define INDICATOR_FOOTER "_INDICATOR"

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

//recursively find the files required, depth first
void get_requires(const std::string& dir, const std::string &filename, std::unordered_map<std::string, int>& depths, std::unordered_map<std::string, int> requires = {}, int depth = 1)
{
	if(requires[filename])
	{
		failed = true;
		std::cout << "dtbuild: error: circular dependency detected in '" << filename << "'!\n";
		return;
	}
	requires[filename] = depth;
	if(!depths[filename] || depths[filename]  <= depth)
	{
		depths[filename] = depth;
	}
	std::vector<std::string> file = read_file(dir + "/indicators/" + filename);
	for (std::string line : file)
	{
		std::vector<std::string> toks = hirzel::tokenize(line, "\t ");
		if (toks[0] == REQUIRE_TOKEN)
		{
			std::string subname = get_filename(purge_delims(toks[1], "\""));
			get_requires(dir, subname, depths, requires, depth + 1);
		}
	}
}

//takes in path to indicators script and returns a pair of its identifier and definition (in that order)
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
			if(toks.size() < 2)
			{
				failed = true;
				std::cout << "dtbuild: error: '" << filename << "': Invalid amount of args in preprocessor directive at line: " << lineIndex << std::endl;
			}

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
				// No need to do anything as compose algo parses this
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

std::string compose_algorithm(const std::string &dir, const std::string &filepath)
{
	std::string buffer
	{
		#include "algorithm.def"
	};
	std::string algo_name, script, indicator_definition_glob, includes, filename;
	std::vector<std::string> input_file;
	std::vector<std::vector<std::string>> indicator_variables;
	std::unordered_map<std::string, int> req_depths;
	std::vector<std::pair<std::string, std::string>> indicator_definitions;

	filename = get_filename(filepath);

	// loading script
	input_file = hirzel::read_file(filepath);
	if (input_file.empty())
	{
		std::cout << "dtbuild: fatal: Failed to open input file!\n";
		failed = true;
		return "";
	}

	/****************************************************
	 * Handling preprocessor/psuedo calls in algorithm	*
	 ****************************************************/
	unsigned int lineIndex = 1;
	for (std::string &s : input_file)
	{
		std::vector<std::string> tokens = hirzel::tokenize(s, " \t");
		if (tokens[0][0] == '$' || tokens[0][0] == '#')
		{
			if(tokens.size() < 2)
			{
				failed = true;
				std::cout << "dtbuild: error: '" << filename << "': Invalid amount of args in preprocessor directive at line: " << lineIndex << std::endl;
			}

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
				std::string indi = get_filename(purge_delims(tokens[1], "\""));
				get_requires(dir, indi, req_depths);
				if(failed)
				{
					return "";
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
				s = "\treturn " + std::to_string(ACTION_NOTHING) + "U;";
		}
		else if (tokens[0] == ACTION_BUY_PROXY)
		{
			s = "\treturn " + std::to_string(ACTION_BUY) + "U;";
		}
		else if (tokens[0] == ACTION_SELL_PROXY)
		{
			s = "\treturn " + std::to_string(ACTION_SELL) + "U;";
		}

		lineIndex++;
	}


	/********************************************
	 * Populating list of indicator definitions	*
	 ********************************************/
	int maxDepth = 0;

	std::unordered_map<std::string, bool> pseudoident;
	for (std::pair<std::string, int> p : req_depths)
	{
		if (maxDepth < p.second)
		{
			maxDepth = p.second;
		}
	}
	for (int i = maxDepth; i > 0; i--)
	{
		for (std::pair<std::string, int> p : req_depths)
		{
			if (p.second == i)
			{
				std::pair<std::string, std::string> indi_def = compose_indicator(dir, p.first);
				pseudoident[indi_def.first] = true;
				indicator_definition_glob += indi_def.second;
			}
		}
	}
	/********************************
	 * Replacing pseudoidentifiers	*
	 ********************************/
	for (std::string& line : input_file)
	{
		std::vector<std::string> line_toks = tokenize(line, " \t\n();");
		if(line_toks.empty())
		{
			continue;
		}
		if(pseudoident[line_toks[0]])
		{
			line = "";
			if(line_toks.size() != 3)
			{
					std::cout << "dtbuild: error: '" << filename << "' Syntax error when declaring indicator: lines " << lineIndex << "!\n";
					failed = true;
					return "";
			}

			indicator_variables.push_back(line_toks);
		}
	}


	//globbing input file into 'script'
	for (std::string s : input_file)
	{
		if(!s.empty())
		{
			script += "\t" + s + "\n";
		}
	}

	if (algo_name.empty())
	{
		std::cout << "dtbuild: fatal: '" << filename << "'A name must be defined!\n";
		failed = true;
		return "";
	}
	
	std::string indi_decl, indi_data, dataset_init;
	for (std::vector<std::string> v : indicator_variables)
	{
		std::string data_var_name = v[1];
		std::string var_name = v[1] + INDICATOR_FOOTER;
		indi_decl += v[0] + " " + var_name + "(" + v[2] + ");\n";
		indi_data += "\n\tconst std::vector<double>& " + data_var_name + " = dataset.at(\"" + data_var_name + "\").data;";
		dataset_init += "\n\tdataset[\"" + data_var_name + "\"] = " + var_name + ".calculate(candles);";
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
	if (argc == 1)
	{
		std::cout << "dtbuid: fatal: No input files!\n";
		return 1;
	}

	std::string odir, cwd, execdir, configpath;
	std::string cxx, cxxflags, lflags;
	std::string filepath;

	cwd = std::filesystem::current_path().string() + "/.";
	std::string rdir = hirzel::get_folder(std::string(argv[0]));
	if(cwd == rdir)
	{
		execdir = rdir;
	}
	else
	{
		execdir = cwd + rdir;
	}
	configpath = execdir + "/" + std::string(CONFIG_FILENAME);

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

	std::vector<std::string> config_file = hirzel::read_file(configpath);

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
	}

	if (cxx.empty())
	{
		std::cout << "dtbuild: fatal: Must supply a c++ compiler in config!\n";
		return 1;
	}

	// std::cout << "SCRIPT FILENAME: " << script_filepath << std::endl;
	std::string algo_folder = hirzel::get_folder(filepath);
	// adding extension on end of file

	std::string basename = hirzel::get_basename(filepath);
	std::string temp_output_filename = algo_folder + "/" + basename + TEMP_OUTPUT_EXTENSION;
	std::string algo_buf = compose_algorithm(algo_folder, filepath);

	if (failed)
	{
		return 1;
	}

	if (printcode)
	{
		unsigned int line = 1;
		printf("%5d | ", line);

		for (char c : algo_buf)
		{
			if(c == '\n')
			{
				putchar(c);
				line++;
				printf("%5d | ", line);
			}
			else
			{
				putchar(c);
			}
		}
		putchar('\n');
	}

	// writing temp file
	hirzel::write_file(temp_output_filename, algo_buf);
	std::string output_name;
	if (odir.empty())
	{
		output_name = algo_folder;
	}
	output_name += odir + "/" + basename + ".so";
	std::string cmd = cxx + " " + cxxflags + temp_output_filename + " -o " + output_name
		+ " -I" + execdir + "/algorithms/include " + lflags;
	// std::cout << "Algo Folder: " << algo_folder << std::endl;
	if (system(cmd.c_str()))
	{
		failed = true;
	}
	if (std::remove(temp_output_filename.c_str()))
	{
		failed = true;
		std::cout << "dtbuild: error: Failed to remove temp files!\n";
	}
	if(failed)
	{
		std::cout << "dtbuild: failed to compile '" + output_name + "'!\n";
	}

	return failed;
}
