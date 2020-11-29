#include "dtbuild.h"

#include <stdio.h>
#include <cctype>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include <hirzel/fileutil.h>
#include <hirzel/strutil.h>

#include "../data/action.h"
#include "lexer.h"

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

#define LABEL_TOKEN "label"
#define IDENTIFIER_TOKEN "identifier"
#define REQUIRE_TOKEN "require"
#define INCLUDE_TOKEN "include"
#define AS_TOKEN "as"

#define ACTION_BUY_PROXY "buy()"
#define ACTION_SELL_PROXY "sell()"
#define ACTION_NOTHING_PROXY "do_nothing()"

#include <filesystem>

using namespace hirzel;

bool failed = false;

std::string mstr =
	R"(
int val()
{
	// this is a comment
	/*
	this is a block
	comment*/
	str s = "HELLO MAN !!!";
	int i = 0;
	i += 2;
	return i;
})";

namespace dtbuild
{
	void syntax_error(const std::string& filepath, const std::string& msg, long line, int col, int width)
	{
		// doing this because they start at 1 in tokens
		line--;
		col--;
		// TODO add open relative to executable by combining filepath with absolute path
		#define RED_ESCAPE "\033[31;1m"
		#define RESET_ESCAPE "\033[0m"
		std::vector<std::string> file = hirzel::file::read_file_as_vector(filepath);
		std::string str = file[line];

		if (width < 1)
		{
			width = str.size() - col;
		}

		std::cout << "dtbuild: " << filepath << ':' << line << ':' << col << ": "
		<< RED_ESCAPE "error: " RESET_ESCAPE << msg << '\n';

		std::cout << std::setw(5) << line + 1 << " | " << str.substr(0, col)
		<< RED_ESCAPE;

		std::cout << str.substr(col, width)
		<< RESET_ESCAPE << str.substr(col + width) << std::endl;

		std::cout << std::setw(5) << " " << " | ";

		for (int i = 1; i < col + 1; i++) std::cout << ' ';
		std::cout << RED_ESCAPE << '^';

		for (int i = 1; i < width; i++) std::cout << '~';
		std::cout << RESET_ESCAPE << '\n';		
	}

	//takes in path to indicators script and returns a pair of its identifier and definition (in that order)
	std::pair<std::string, std::string> compose_indicator(const std::string &dir, const std::string &filename)
	{
		std::string buffer{
#include "indicator.def"
		};

		std::string label, script, classname, includes;
		std::vector<std::string> file = file::read_file_as_vector(dir + "/indicators/" + filename);

		if (file.empty())
		{
			std::cout << "dtbuild: error: Failed to load file '" + filename + "'\n";
			failed = true;
			return {"", ""};
		}

		script += '\n';

		unsigned lineIndex = 1;
		for (std::string s : file)
		{
			std::vector<std::string> toks = str::tokenize(s, "\t ");
			if (toks[0][0] == '$' || toks[0][0] == '#')
			{
				if (toks.size() < 2)
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
						return {"", ""};
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

		str::find_and_replace(buffer, INCLUDES_HOOK, includes);
		str::find_and_replace(buffer, INDICATOR_CLASS_HOOK, classname);
		str::find_and_replace(buffer, INDICATOR_LABEL_HOOK, label);
		str::find_and_replace(buffer, CALCULATE_SCRIPT_HOOK, script);

		return {classname, buffer};
	}

	/*************************
 	 *   Compose Algorithm   *
 	 *************************/

	std::string compose_algorithm(const std::string &dir, const std::string &filepath)
	{
		using namespace daytrender;
		std::string buffer{
#include "algorithm.def"
		};
		std::string algo_name, script, indicator_definition_glob, includes, filename, action_glob;
		std::vector<std::string> input_file, requires;
		std::vector<std::vector<std::string>> indicator_variables;
		std::unordered_map<std::string, int> req_depths;
		std::vector<std::pair<std::string, std::string>> indicator_definitions;
		tokenlist tokens;
		filename = str::get_filename(filepath);

		// inserting action definitions at top of output file
		action_glob += "#define " ACTION_NOTHING_PROXY " return " + std::to_string(ACTION_NOTHING) + "\n";
		action_glob += "#define " ACTION_SELL_PROXY " return " + std::to_string(ACTION_SELL) + "\n";
		action_glob += "#define " ACTION_BUY_PROXY " return " + std::to_string(ACTION_BUY) + "\n\n";
		buffer.insert(0, action_glob);

		script = file::read_file_as_string(filepath);
		if (script.empty())
		{
			std::cout << "dtbuild: fatal: Failed to open the input file!\n";
		}

		tokens = lex(script, filepath);
		if (tokens.empty())
		{
			return "";
		}

		std::cout << "\n*********************************\nTokens received\n*********************************\n\n";

		syntax_error(filepath, "invalid preprocessor keyword", tokens[1].line, tokens[1].column-1, tokens[1].value.size()+1);
		/****************************************************
	 * Handling preprocessor/psuedo calls in algorithm	*
	 ****************************************************/

		unsigned line = 1;
		bool error = false;
		for (unsigned i = 0; i < tokens.size(); i++)
		{
			switch (tokens[i].type)
			{

			case DOLLAR_SIGN:
				i++;
				if (tokens[i].value == LABEL_TOKEN)
				{
					if (!algo_name.empty())
					{
						std::cout << "dtbuild: error: redefinition of algorithm label: line " << line << "\n";
						return "";
					}
					i++;
					algo_name = tokens[i].value;
					std::cout << "ALGO NAME: "
							  << "'" << algo_name << "'" << std::endl;
				}
				else
				{
					std::cout << "dtbuild: error: invalid keyword '$" << tokens[i].value << "' used: line " << line << "\n";
					return "";
				}
				break;
			// potential invalid memory access
			case POUND_SIGN:
				i++;
				if (tokens[i].value == REQUIRE_TOKEN)
				{	
					i++;
					if (tokens[i].type != STRING_LITERAL)
					{
						syntax_error(filepath, "invalid argument type for preprocessor directive",
							tokens[i].line, tokens[i].column, tokens[i].value.size());
							return "";
					}
					std::string filepath = tokens[i].value;
					requires.push_back(filepath);
				}
				else if (tokens[i].value == INCLUDE_TOKEN)
				{
					i++;

					// adding the tokens to the include glob
					includes += tokens[i - 1].value;
					while (tokens[i].value != "\n")
					{
						includes += tokens[i].value + ' ';

						i++;
					}
					includes += tokens[i].value;
				}
				else
				{
					//syntax_error(filepath, "invalid preprocessor directive", 
				}
				break;
			}
		}
		// aborting if there was no name defined in algo
		if (algo_name.empty())
		{
			std::cout << "dtbuild: fatal: '" << filename << "'A name must be defined!\n";
			return "";
		}

		/********************************************
	 	 * Populating list of indicator definitions	*
	 	 ********************************************/

		/************************************************************
	 	 *	Parsing indicator declarations and cleaning up tokens	*
	 	 ************************************************************/
		unsigned offset = 0;
		line = 1;
		for (unsigned i = 0; i < tokens.size(); i++)
		{
			switch (tokens[i].type)
			{

			case DOLLAR_SIGN:
				/*
				while (tokens[i].type != NEW_LINE)
				{
					offset++;
					i++;
				}
				*/
				// curr token is a newline
				line++;
				offset++;
				continue;

			case POUND_SIGN:
				if (tokens[i].value == REQUIRE_TOKEN)
				{
					offset += 2;
					i += 2;
					/*
					while (tokens[i].type != NEW_LINE)
					{
						offset++;
						i++;
					}
					*/
					line++;
					offset++;
					continue;
				}
				break;
			}
			/*
		if (pseudoident[script_toks[i]])
		{
			std::cout << "PSEUDOIDENT: " << script_toks[i] << std::endl;
			std::vector<std::string> var_toks;
			while (script_toks[i] != ";")
			{
				var_toks.push_back(script_toks[i]);
				offset++;
				i++;
			}
			indicator_variables.push_back(var_toks);
			offset++;
			continue;
		}
		*/
			tokens[i - offset] = tokens[i];
		}

		//tokens.resize(tokens.size() - offset);

		return "";
		script.clear();
		/*
	unsigned tabs_level = 1;
	for (unsigned i = 0; i < tokens.size(); i++)
	{
		const std::string& t = tokens[i].value;
		switch(t[0])
		{
			case '(':
				if (script.back() == ' ') script.pop_back();
				script += t;
				break;

			case ')':
				if (script.back() == ' ') script.pop_back();
				script += t;
				break;

			case '{':
				script += '\n' + t;
				tabs_level++;
				break;

			case '}':
				script.pop_back();
				script += t + '\n';
				tabs_level--;
				break;
			case ';':
				if(script.back() == ' ') script.pop_back();
				script += t;
				break;
			case '\n':
				script += t;
				for (unsigned i = 0; i < tabs_level; i++) script += '\t';
				break;

			default:
				script += t + ' ';
				break;
		}
	}
	std::cout << "Script:\n" << script << std::endl;
*/

		return "";
		/************************************************
	 * 	Converting pseudoidentifiers into real data	*
	 ************************************************/

		std::string indi_decl, indi_data, dataset_init;
		for (const std::vector<std::string> &v : indicator_variables)
		{
			std::string data_var_name = v[1];
			std::string var_name = v[1] + INDICATOR_FOOTER;
			indi_decl += v[0] + " " + var_name;
			for (unsigned i = 2; i < v.size(); i++)
			{
				indi_decl += v[i];
				if (v[i] == ",")
				{
					indi_decl += " ";
				}
			}
			indi_decl += "\n";

			indi_data += "\n\tconst std::vector<double>& " + data_var_name + " = dataset.at(\"" + data_var_name + "\").data;";
			dataset_init += "\n\tdataset[\"" + data_var_name + "\"] = " + var_name + ".calculate(candles);";
		}

		// replacing hooks in output buffer
		str::find_and_replace(buffer, ALGORITHM_NAME_HOOK, algo_name);
		str::find_and_replace(buffer, INCLUDES_HOOK, includes);
		str::find_and_replace(buffer, INDICATOR_DEFINE_HOOK, indicator_definition_glob);
		str::find_and_replace(buffer, INDICATOR_DECLARE_HOOK, indi_decl);
		str::find_and_replace(buffer, INDICATOR_DATA_HOOK, indi_data);
		str::find_and_replace(buffer, ALGORITHM_SCRIPT_HOOK, script);
		str::find_and_replace(buffer, DATASET_INIT_HOOK, dataset_init);

		return buffer;
	}
}

int main(int argc, char *argv[])
{
	using namespace dtbuild;

	if (argc == 1)
	{
		std::cout << "dtbuid: fatal: No input files!\n";
		return 1;
	}

	std::string odir, cwd, execdir, configpath;
	std::string cxx, cxxflags, lflags;
	std::string filepath;

	cwd = std::filesystem::current_path().string() + "/.";
	std::string rdir = str::get_folder(std::string(argv[0]));
	if (cwd == rdir)
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
			if (filepath.empty())
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

	std::vector<std::string> config_file = file::read_file_as_vector(configpath);

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
		std::vector<std::string> tokens = str::tokenize(s, " \t=");
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
	std::string algo_folder = str::get_folder(filepath);
	// adding extension on end of file

	std::string basename = str::get_basename(filepath);
	std::string temp_output_filename = algo_folder + "/" + basename + TEMP_OUTPUT_EXTENSION;
	std::string algo_buf = compose_algorithm(algo_folder, filepath);

	if (algo_buf.empty())
	{
		return 1;
	}

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
			if (c == '\n')
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
	file::write_file(temp_output_filename, algo_buf);
	std::string output_name;
	if (odir.empty())
	{
		output_name = algo_folder;
	}
	output_name += odir + "/" + basename + ".so";
	std::string cmd = cxx + " " + cxxflags + temp_output_filename + " -o " + output_name + " -I" + execdir + "/algorithms/include " + lflags;
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
	if (failed)
	{
		std::cout << "dtbuild: failed to compile '" + output_name + "'!\n";
	}

	return failed;
}
