#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cctype>
#include <filesystem>
#include <map>

#include "../util/fileutil.h"
#include "../util/strutil.h"

#define TEMP_OUTPUT_EXTENSION	".cpp"
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
#define CANDLE_DEFINE_HOOK		"%CANDLE_DEFINE%"
#define DATASET_INIT_HOOK		"%DATASET_INIT%"
#define INDICATOR_DATA_HOOK		"%INDICATOR_DATA%"
#define ACTION_DEFINE_HOOK		"%ACTION_DEFINE%"

// These occur in indicator.def
#define INDICATOR_CLASS_HOOK	"%INDICATOR_CLASS%"
#define INDICATOR_LABEL_HOOK	"%INDICATOR_LABEL%"
#define CALCULATE_SCRIPT_HOOK	"%CALCULATE_SCRIPT%"

#define LABEL_TOKEN				"$label"
#define REQUIRE_TOKEN			"$require"
#define INCLUDE_TOKEN			"$include"
#define AS_TOKEN				"as"

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		std::cout << "dtbuid: fatal: No input files!\n";
		return 1;
	}

	std::string odir, indidef, algodef, candledef, candle_h, candle_cpp;
	std::string cxx, cxxflags, lflags, srcdir, action_h;
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

	if(odir.empty())
	{
		odir = ".";
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

	indidef = hirzel::read_rawstring(srcdir + "/dtbuild/indicator.def");
	algodef = hirzel::read_rawstring(srcdir + "/dtbuild/algorithm.def");

	candle_h = hirzel::read_rawstring(srcdir + "/data/candle.h");
	hirzel::replace_hook(candle_h, "#pragma once", " ");

	candle_cpp = hirzel::read_rawstring(srcdir + "/data/candle.cpp");
	hirzel::replace_hook(candle_cpp, "#include \"candle.h\"", " ");

	action_h = hirzel::read_rawstring(srcdir + "/data/action.h");
	hirzel::replace_hook(action_h, "#pragma once", " ");

	candledef = candle_h + "\n" + candle_cpp;

	for (std::string script_filename : filenames)
	{
		std::string cwd, algo_name;
		std::vector<std::string> input_file;

		//finding current working directory
		unsigned int slashIndex = -1;
		for (unsigned int i = 0; i < script_filename.size(); i++)
		{
			if(script_filename[i] == '/')
			{
				slashIndex = i;
			}
		}
		if(slashIndex < script_filename.size())
		{
			for (unsigned int i = 0; i < slashIndex; i++)
			{
				cwd = script_filename.substr(0, slashIndex);
			}
		}
		else
		{
			cwd = ".";
		}

		// loading script
		input_file = hirzel::read_file(script_filename);

		if (input_file.empty())
		{
			std::cout << "dtbuild: fatal: Failed to open input file!\n";
			return 1;
		}
		
		// first string is name, second string is filepath
		std::vector<std::pair<std::string, std::string>> indicators;
		std::vector<std::vector<std::string>> indicator_declarations;

		unsigned int lineIndex = 0;
		for (std::string &s : input_file)
		{
			std::vector<std::string> tokens = hirzel::tokenize(s, " ");

			if(tokens[0][0] == '$')
			{
				if(tokens[0] == LABEL_TOKEN)
				{
					if(!algo_name.empty())
					{
						std::cout << "dtbuild: error: Redefinition of name!\n";
						return 1;
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
				else if (tokens[0] == REQUIRE_TOKEN && tokens[2] == AS_TOKEN)
				{
					std::string filepath = hirzel::purge_delims(tokens[1], "\"");

					if (filepath[0] != '/')
					{
						filepath = cwd + "/" + filepath;
					}

					indicators.push_back({ tokens[3], filepath });
				}
				else if (tokens[0] == INCLUDE_TOKEN)
				{
					// TODO : allow for including normal c++ files
				}
				else
				{
					std::cout << "dtbuild: error: Syntax error: line " << lineIndex << "!\n";
				}
				
				s = "";
			}
			else
			{
				for (std::pair<std::string, std::string> pair : indicators)
				{
					//if the identifier used matches a declared indicator
					if(tokens[0] == pair.first)
					{
						std::vector<std::string> vartoks = hirzel::tokenize(s, " \t();");
						if(vartoks.size() != 3)
						{
							std::cout << "dtbuild: error: Syntax error when declaring indicator: lines " << lineIndex << "!\n";
							return 1;
						}
						indicator_declarations.push_back(vartoks);
						s = "";
						//break;
					}
				}
			}

			lineIndex++;
		}

		if(algo_name.empty())
		{
			std::cout << "dtbuild: fatal: A name must be defined!\n";
			return 1;
		}

		std::string algo_buf, algo_script, indi_def;
		algo_buf = algodef;

		// composing each indicator required in file
		for (std::pair<std::string, std::string> pair : indicators)
		{
			std::string indi_label, indicator_buf, indi_script, indi_class;
			std::vector<std::string> indicator_file;

			indicator_file = hirzel::read_file(pair.second);
			indicator_buf = indidef;
			indi_class = pair.first;

			if(indicator_file.empty())
			{
				std::cout << "dtbuild: error: Failed to load file '" + pair.second + "'\n";
				return 1;
			}

			indi_script += "\n";
			lineIndex = 0;
			for (std::string s : indicator_file)
			{
				if (s[0] == '$')
				{
					std::vector<std::string> toks = hirzel::tokenize(s, " ");

					if(toks[0] == LABEL_TOKEN)
					{
						indi_label = hirzel::purge_delims(toks[1], "\"");
					}
					else if (toks[0] == REQUIRE_TOKEN && toks[2] == AS_TOKEN)
					{
						// TODO : allow for including other indicators
					}
					else if (toks[0] == INCLUDE_TOKEN)
					{
						// TODO : allow for including normal c++ files
					}
					else
					{
						std::cout << "dtbuild: error: Syntax error: line " << lineIndex << std::endl;
					}

					lineIndex++;
					continue;
				}

				indi_script += "\n\t\t" + s;
				lineIndex++;
			}

			if (indi_label.empty())
			{
				std::cout << "dtbuild: error: $label must be defined in indicator '" + pair.first + "'\n";
				return 1;
			}

			hirzel::replace_hook(indicator_buf, INDICATOR_CLASS_HOOK, indi_class);
			hirzel::replace_hook(indicator_buf, INDICATOR_LABEL_HOOK, indi_label);
			hirzel::replace_hook(indicator_buf, CALCULATE_SCRIPT_HOOK, indi_script);

			indi_def += indicator_buf;
		}

		// adding to script string, mainly for human readability
		for (unsigned int i = 0; i < input_file.size(); i++)
		{
			algo_script += "\n\t" + input_file[i];
		}

		std::string indi_decl, indi_data, dataset_init;

		for (std::vector<std::string> v : indicator_declarations)
		{
			std::string data_var_name = v[1];
			std::string var_name = v[1] + INDICATOR_FOOTER;

			indi_decl += v[0] + " " + var_name + "(" + v[2] + ");\n";
			indi_data += "\n\tstd::vector<double> " + data_var_name + " = dataset.at(\"" + data_var_name+ "\").second;";
			dataset_init += "\n\tdataset[\"" + data_var_name + "\"] = " + var_name +".calculate(candles, index, window);";
		}

		// replacing hooks in output buffer
		hirzel::replace_hook(algo_buf, ALGORITHM_NAME_HOOK, algo_name);
		hirzel::replace_hook(algo_buf, CANDLE_DEFINE_HOOK, candledef);
		hirzel::replace_hook(algo_buf, ALGORITHM_SCRIPT_HOOK, algo_script);
		hirzel::replace_hook(algo_buf, INDICATOR_DEFINE_HOOK, indi_def);
		hirzel::replace_hook(algo_buf, ACTION_DEFINE_HOOK, action_h);
		hirzel::replace_hook(algo_buf, INDICATOR_DECLARE_HOOK, indi_decl);
		hirzel::replace_hook(algo_buf, INDICATOR_DATA_HOOK, indi_data);
		hirzel::replace_hook(algo_buf, DATASET_INIT_HOOK, dataset_init);

		// adding extension on end of file
		size_t dotpos = script_filename.find(".");
		std::string basename = script_filename.substr(0, dotpos);
		std::string temp_output_filename = basename + TEMP_OUTPUT_EXTENSION;
		//std::cout << "OUTPUT FILENAME: " << temp_output_filename << std::endl;

		// writing temp file
		hirzel::write_file(temp_output_filename, algo_buf);
		
		std::string output_name = odir + "/" + basename + ".so";
		std::string cmd = cxx + " " + cxxflags + temp_output_filename
			+ " -o " + output_name + " " + lflags;

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
	}

	return 0;
}
