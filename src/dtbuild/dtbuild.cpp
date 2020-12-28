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
#include "preprocessor.h"

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
	std::string cwd, execdir;

	std::string syntax_error(const std::string& filepath, const std::string& msg, long line, int col, int width)
	{
		col--;
		// TODO add open relative to executable by combining filepath with absolute path
		#define RED_ESCAPE "\033[31;1m"
		#define RESET_ESCAPE "\033[0m"
		std::vector<std::string> file = hirzel::file::read_file_as_vector(filepath);
		std::string str = file[line - 1];
		std::string out;
		if (width < 1)
		{
			width = str.size() - col;
		}

		std::string l = std::to_string(line);

		out += "dtbuild: ";
		out += filepath;
		out += ':';
		out += l;
		out += ':';
		out += std::to_string(col + 1);
		out += ": " RED_ESCAPE "error: " RESET_ESCAPE;
		out += msg;
		out += '\n';

		for (int i = 0; i < 5 - l.size(); i++) out += ' ';
		out += l;
		out += " | ";
		out += str.substr(0, col);
		out += RED_ESCAPE;

		out += str.substr(col, width);
		out += RESET_ESCAPE;
		out += str.substr(col + width);
		out += "\n      | ";

		for (int i = 0; i < col; i++)
		{
			if (str[i] < 33)
			{
				out += str[i];
			}
			else
			{
				out += ' ';
			}
		}
		out += RED_ESCAPE "^";

		for (int i = 1; i < width; i++) out += '~';
		out += RESET_ESCAPE "\n\n";
		return out;	
	}

	std::string tabs(int amt)
	{
		std::string out;
		for (int i = 0; i < amt; i++)
		{
			out += '\t';
		}
		return out;
	}

	std::string transpile(const std::string &dir, const std::string &filepath)
	{
		using namespace daytrender;

		std::string script;
		std::vector<Token> tokens;
		Node program;

		lex_init();
		parse_init();

		std::cout << "\n*********************************\nTokenizing source code\n*********************************\n\n";

		tokens = lex(filepath);

		if (tokens.empty())
		{
			std::cout << "Token list was empty!\n";
			return "";
		}

		for (long i = 0; i < tokens.size(); i++)
		{
			std::cout << i << ": " << tokens[i] << std::endl;
		}

		std::cout << "\n*********************************\nParsing tokens\n*********************************\n\n";

		/**********************
	 	 *   Parsing tokens   *
	 	 **********************/
		
		program = parse(tokens, filepath);
		if (program.empty())
		{
			return "";
		}

		std::cout << "\n*********************************\nGenerating code\n*********************************\n\n";

		std::string code = generate_code(program);

		std::cout << "\nGenerated C++ Source:\n\n" << code;

		return "";
		return code;
	}
}

int main(int argc, char *argv[])
{
	std::cout << "argv[0]: " << argv[0] << std::endl;

	using namespace dtbuild;

	if (argc == 1)
	{
		std::cout << "dtbuid: fatal: No input files!\n";
		return 1;
	}

	std::string odir, configpath;
	std::string cxx, cxxflags, lflags;
	std::string filepath;

	cwd = std::filesystem::current_path().string() + "/";
	std::string rdir = str::get_folder(std::string(argv[0]));
	execdir = (cwd == rdir) ? rdir : cwd + rdir;
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
	std::string algo_buf = transpile(algo_folder, filepath);

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
