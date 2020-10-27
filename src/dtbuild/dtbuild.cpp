#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cctype>
#include <filesystem>
#include <map>

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>
#include "../data/action.h"

#define TEMP_OUTPUT_EXTENSION	".cpp"

#if defined(_WIN32) || defined(_WIN64)
#define OUTPUT_EXTENSION		".dll"
#elif defined(linux) || defined (__unix__)
#define OUTPUT_EXTENSION		".so"
#endif

#define CONFIG_FILENAME			"./dtbuild.config"
#define ALGODEF_FILENAME		"algorithm.def"
#define INDIDEF_FILENAME		"indicator.def"
#define INDICATOR_FOOTER		"_INDICATOR"

// hooks for replacement

// These occur in algorithm.def
#define ALGORITHM_NAME_HOOK		"%ALGORITHM_NAME%"
#define ALGORITHM_SCRIPT_HOOK	"%ALGORITHM_SCRIPT%"
#define INDICATOR_DEFINE_HOOK	"%INDICATOR_DEFINE%"
#define INDICATOR_DECLARE_HOOK	"%INDICATOR_DECLARE%"
#define DATASET_INIT_HOOK		"%DATASET_INIT%"
#define INDICATOR_DATA_HOOK		"%INDICATOR_DATA%"
#define INCLUDES_HOOK			"%INCLUDES%"

// These occur in indicator.def
#define INDICATOR_CLASS_HOOK	"%INDICATOR_CLASS%"
#define INDICATOR_LABEL_HOOK	"%INDICATOR_LABEL%"
#define CALCULATE_SCRIPT_HOOK	"%CALCULATE_SCRIPT%"

#define LABEL_TOKEN				"$label"
#define IDENTIFIER_TOKEN		"$identifier"
#define REQUIRE_TOKEN			"$require"
#define INCLUDE_TOKEN			"$include"
#define AS_TOKEN				"as"

#define ACTION_BUY_PROXY		"buy();"
#define ACTION_SELL_PROXY		"sell();"
#define ACTION_NOTHING_PROXY	"null();"

std::string indicator_template, algorithm_template, candle_definition, action_h;

//takes in path to indicators script and returns a pair of its identifier and definition
std::pair<std::string, std::string> compose_indicator(const std::string& filename)
{
	std::string label, buffer, script, classname;
	std::vector<std::string> file;
	file = hirzel::read_file(filename);
	buffer = indicator_template;

	if(file.empty())
	{
		std::cout << "dtbuild: error: Failed to load file '" + filename + "'\n";
		return { "", "" };
	}

	script += "\n";

	unsigned int lineIndex = 0;
	for (std::string s : file)
	{
		if (s[0] == '$')
		{
			std::vector<std::string> toks = hirzel::tokenize(s, " ");
			if(toks[0] == LABEL_TOKEN)
			{
				label = hirzel::purge_delims(toks[1], "\"");
			}
			else if (toks[0] == IDENTIFIER_TOKEN)
			{
				classname = toks[1];
			}
			//else if (toks[0] == REQUIRE_TOKEN)
			//{
				// TODO : allow for including other indicators
			//}
			//else if (toks[0] == INCLUDE_TOKEN)
			//{
				// TODO : allow for including normal c++ files
			//}
			else
			{
				std::cout << "dtbuild: error: Syntax error: line " << lineIndex << std::endl;
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
		return { "", ""};
	}
	hirzel::replace_hook(buffer, INDICATOR_CLASS_HOOK, classname);
	hirzel::replace_hook(buffer, INDICATOR_LABEL_HOOK, label);
	hirzel::replace_hook(buffer, CALCULATE_SCRIPT_HOOK, script);
	return { classname, buffer };
}


/*
	TODO:
		---Add include guards to required files so that no algorithm or indicator double
		requires a file. This may require passing required_filenames into compose_indicator
		so that each indicator knows as well considering they will be able to require
		as well.

		---Add including normal cpp files to indicators.
*/


std::string compose_algorithm(const std::string& dir, const std::string& filepath)
{
	std::string algo_name, buffer, script, indicator_definition_glob;
	std::vector<std::string> input_file, required_filenames;
	std::vector<std::vector<std::string>> indicator_variables;
	std::vector<std::pair<std::string, std::string>> indicator_definitions;

	// loading script
	input_file = hirzel::read_file(filepath);
	if (input_file.empty())
	{
		std::cout << "dtbuild: fatal: Failed to open input file!\n";
		return "";
	}

	unsigned int lineIndex = 0;
	for (std::string &s : input_file)
	{
		std::vector<std::string> tokens = hirzel::tokenize(s, " \t");

		if(tokens[0][0] == '$')
		{
			if(tokens[0] == LABEL_TOKEN)
			{
				if(!algo_name.empty())
				{
					std::cout << "dtbuild: error: Redefinition of name!\n";
					return "";
				}
				int startIndex = -1, endIndex = -1;
				for (unsigned int i = 0; i < s.size(); i++)
				{
					if(s[i] == '"')
					{
						if(startIndex == -1)
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
				std::string filepath = hirzel::purge_delims(tokens[1], "\"");
				if (filepath[0] != '/')
				{
					filepath = dir + "/" + filepath;
				}

				indicator_definitions.push_back(compose_indicator(filepath));
			}
			//else if (tokens[0] == INCLUDE_TOKEN)
			//{
				// TODO : allow for including normal c++ files
			//}
			else
			{
				std::cout << "dtbuild: error: Syntax error: line " << lineIndex << "!\n";
			}
				
			s = "";
		}
		else
		{
			if (tokens[0] == ACTION_NOTHING_PROXY)
			{
				s = "\treturn " + std::to_string(ACTION_NOTHING) + ";";
			}
			else if(tokens[0] == ACTION_BUY_PROXY)
			{
				s = "\treturn " + std::to_string(ACTION_BUY) + ";";
			}
			else if (tokens[0] == ACTION_SELL_PROXY)
			{
				s = "\treturn " + std::to_string(ACTION_SELL) + ";";
			}
			else
			{
				for (std::pair<std::string, std::string> pair : indicator_definitions)
				{
				//if the identifier used matches a declared indicator
					if(tokens[0] == pair.first)
					{
						std::vector<std::string> vartoks = hirzel::tokenize(s, " \t();");
						if(vartoks.size() != 3)
						{
							std::cout << "dtbuild: error: Syntax error when declaring indicator: lines " << lineIndex << "!\n";
							return "";
						}

						indicator_variables.push_back(vartoks);
						s = "";
					}
				}
			}
		}

		lineIndex++;
	}

	if(algo_name.empty())
	{
		std::cout << "dtbuild: fatal: A name must be defined!\n";
		return "";
	}

	buffer = algorithm_template;
	// composing each indicator required in file
	for (std::pair<std::string, std::string> pair : indicator_definitions)
	{
		indicator_definition_glob += pair.second + "\n";
	}

	// adding to script string, mainly for human readability
	for (unsigned int i = 0; i < input_file.size(); i++)
	{
		script += "\n\t" + input_file[i];
	}

	std::string indi_decl, indi_data, dataset_init;
	for (std::vector<std::string> v : indicator_variables)
	{
		std::string data_var_name = v[1];
		std::string var_name = v[1] + INDICATOR_FOOTER;
		indi_decl += v[0] + " " + var_name + "(" + v[2] + ");\n";
		indi_data += "\n\tstd::vector<double> " + data_var_name + " = dataset.at(\"" + data_var_name+ "\").second;";
		dataset_init += "\n\tdataset[\"" + data_var_name + "\"] = " + var_name +".calculate(candles, index, window);";
	}
	// replacing hooks in output buffer
	hirzel::replace_hook(buffer, ALGORITHM_NAME_HOOK, algo_name);
	hirzel::replace_hook(buffer, CANDLE_DEFINE_HOOK, candle_definition);
	hirzel::replace_hook(buffer, ALGORITHM_SCRIPT_HOOK, script);
	hirzel::replace_hook(buffer, INDICATOR_DEFINE_HOOK, indicator_definition_glob);
	hirzel::replace_hook(buffer, ACTION_DEFINE_HOOK, action_h);
	hirzel::replace_hook(buffer, INDICATOR_DECLARE_HOOK, indi_decl);
	hirzel::replace_hook(buffer, INDICATOR_DATA_HOOK, indi_data);
	hirzel::replace_hook(buffer, DATASET_INIT_HOOK, dataset_init);

	return buffer;
}



int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		std::cout << "dtbuid: fatal: No input files!\n";
		return 1;
	}

	std::string odir, candle_h, candle_cpp;
	std::string cxx, cxxflags, lflags, srcdir;
	std::vector<std::string> filenames;

	bool setodir = false;
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if(arg == "-o")
		{
			setodir = true;
			continue;
		}

		if (setodir)
		{
			odir = argv[i];
			setodir = false;
		}
		else
		{
			filenames.push_back(argv[i]);
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
			for(unsigned int i = 1; i < tokens.size(); i++)
			{
				cxxflags += tokens[i] + " ";
			}
		}
		else if (tokens[0] == "LFLAGS")
		{
			for(unsigned int i = 1; i < tokens.size(); i++)
			{
				lflags += tokens[i] + " ";
			}
		}
		else if (tokens[0] == "DTSRCDIR")
		{
			srcdir = tokens[1];
		}
	}

	if(srcdir.empty())
	{
		std::cout << "dtbuild: fatal: Must supply path of DayTrender src folder in config!\n";
		return 1;
	}

	if(cxx.empty())
	{
		std::cout << "dtbuild: fatal: Must supply a c++ compiler in config!\n";
		return 1;
	}

	indicator_template = hirzel::read_rawstring(srcdir + "/dtbuild/indicator.def");
	algorithm_template = hirzel::read_rawstring(srcdir + "/dtbuild/algorithm.def");

	candle_h = hirzel::read_rawstring(srcdir + "/data/candle.h");
	hirzel::replace_hook(candle_h, "#pragma once", " ");

	candle_cpp = hirzel::read_rawstring(srcdir + "/data/candle.cpp");
	hirzel::replace_hook(candle_cpp, "#include \"candle.h\"", " ");

	action_h = hirzel::read_rawstring(srcdir + "/data/action.h");
	hirzel::replace_hook(action_h, "#pragma once", " ");

	candle_definition = candle_h + "\n" + candle_cpp;

	for (std::string script_filepath : filenames)
	{
		// std::cout << "SCRIPT FILENAME: " << script_filepath << std::endl;
		std::string cwd = hirzel::get_folder(script_filepath);

		// adding extension on end of file
		std::string basename = hirzel::get_basename(script_filepath);
		std::string temp_output_filename = cwd + "/" + basename + TEMP_OUTPUT_EXTENSION;

		std::string algo_buf = compose_algorithm(cwd, script_filepath);

		std::cout << "OUTPUT:\n" << algo_buf << std::endl;

		// writing temp file
		hirzel::write_file(temp_output_filename, algo_buf);
		std::string output_name;

		if(odir.empty())
		{
			output_name = cwd;
		}

		output_name += odir + "/" + basename + ".so";
		std::string cmd = cxx + " " + cxxflags + temp_output_filename
			+ " -o " + output_name + " " + lflags;

		// std::cout << "CWD: " << cwd << std::endl;
		std::cout << "Now compiling...\n";
		int ret = system(cmd.c_str());
		std::cout << "Deleting temp files...\n";
		int del = std::remove(temp_output_filename.c_str());

		if(del)
		{
			std::cout << "dtbuild: error: Failed to remove temp files!\n";
		}

		if(!ret)
		{
			std::cout << "Successfully compiled " + output_name << std::endl;
		}
		else
		{
			std::cout << "Failed to compile " + output_name + "!\n";
		}		
	}

	return 0;
}
