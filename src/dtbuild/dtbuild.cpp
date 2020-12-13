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
#include "parser.h"
#include "generator.h"

#define TEMP_OUTPUT_EXTENSION ".cpp"

#if defined(_WIN32) || defined(_WIN64)
#define OUTPUT_EXTENSION ".dll"
#elif defined(linux) || defined(__unix__)
#define OUTPUT_EXTENSION ".so"
#endif

#define CONFIG_FILENAME "config/dtbuild.config"
#define ALGODEF_FILENAME "algorithm.def"
#define INDIDEF_FILENAME "indicator.def"

#include <filesystem>

using namespace hirzel;

namespace dtbuild
{
	void syntax_error(const std::string& filepath, const std::string& msg, long line, int col, int width)
	{
		col--;
		// TODO add open relative to executable by combining filepath with absolute path
		#define RED_ESCAPE "\033[31;1m"
		#define RESET_ESCAPE "\033[0m"
		std::vector<std::string> file = hirzel::file::read_file_as_vector(filepath);
		std::string str = file[line - 1];

		if (width < 1)
		{
			width = str.size() - col;
		}

		std::cout << "dtbuild: " << filepath << ':' << line << ':' << col + 1 << ": "
		<< RED_ESCAPE "error: " RESET_ESCAPE << msg << '\n';

		std::cout << std::setw(5) << line << " | " << str.substr(0, col)
		<< RED_ESCAPE;

		std::cout << str.substr(col, width)
		<< RESET_ESCAPE << str.substr(col + width) << std::endl;

		std::cout << std::setw(5) << " " << " | ";

		for (int i = 0; i < col; i++)
		{
			if (str[i] < 33)
			{
				std::cout << str[i];
			}
			else
			{
				std::cout << ' ';
			}
		}
		std::cout << RED_ESCAPE << '^';

		for (int i = 1; i < width; i++) std::cout << '~';
		std::cout << RESET_ESCAPE << "\n\n";		
	}

	/*************************
 	 *   Compose Algorithm   *
 	 *************************/

	std::string compose_algorithm(const std::string &dir, const std::string &filepath)
	{
		using namespace daytrender;
		std::string buffer
		{
			#include "algorithm.def"
		};
		std::string algo_name, script, indicator_definition_glob, includes, filename, action_glob;
		std::vector<std::string> input_file, requires;
		std::vector<std::vector<std::string>> indicator_variables;
		std::unordered_map<std::string, int> req_depths;
		std::vector<std::pair<std::string, std::string>> indicator_definitions;
		lexer::tokenlist tokens;
		parser::Node program;
		filename = str::get_filename(filepath);

		lexer::init();
		parser::init();

		script = file::read_file_as_string(filepath);
		if (script.empty())
		{
			std::cout << "dtbuild: fatal: Failed to open the input file!\n";
		}

		std::cout << "\n*********************************\nTokenizing source code\n*********************************\n\n";

		tokens = lexer::lex(script, filepath);
		if (tokens.empty())
		{
			return "";
		}

		

		std::cout << "\n*********************************\nParsing tokens\n*********************************\n\n";

		/**********************
	 	 *   Parsing tokens   *
	 	 **********************/
		
		program = parser::parse(tokens, filepath);
		if (program.empty())
		{
			return "";
		}

		std::cout << "\n*********************************\nGenerating code\n*********************************\n\n";

		std::string code = generator::generate_code(program);

		std::cout << "\nGenerated C++ Source:\n\n" << code;

		return "";
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

	std::string algo_folder = str::get_folder(filepath);
	// adding extension on end of file

	std::string basename = str::get_basename(filepath);
	std::string temp_output_filename = algo_folder + "/" + basename + TEMP_OUTPUT_EXTENSION;
	std::string algo_buf = compose_algorithm(algo_folder, filepath);

	if (algo_buf.empty())
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
	bool failed = false;
	if (system(cmd.c_str()))
	{
		failed = true;
	}
	if (std::remove(temp_output_filename.c_str()))
	{
		failed = true;
		std::cout << "dtbuild: error: Failed to remove temp files!\n";
	}
	return failed;
}
