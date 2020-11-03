#include "fileutil.h"

#include <iostream>
#include <fstream>

#include "strutil.h"

namespace hirzel
{
	std::vector<std::string> read_file(const std::string& filename)
	{
		std::string line;
		std::vector<std::string> lines;
		std::ifstream fin(filename);
		
		if (fin.is_open())
		{
			while (std::getline(fin, line))
			{
				if(line.empty())
				{
					continue;
				}
				lines.push_back(line);
			}
			fin.close();
		}
		else
		{
			#ifdef HXUTILS_DEBUG
			std::cout << "FileUtil::read_file() : Failed to open '" + filename + "'" << std::endl;
			#endif
		}
		return lines;
	}
	
	std::string read_string(const std::string& filename)
	{
		std::string text, line;
		std::ifstream fin(filename);
		
		if (fin.is_open())
		{
			while (std::getline(fin, line))
			{
				if(line.empty())
				{
					continue;
				}
				text += line;
			}
			fin.close();
		}
		else
		{
			#ifdef HXUTILS_DEBUG
			std::cout << "FileUtil::read_file_as_string() : Failed to open '" + filename + "'" << std::endl;
			#endif
		}
		return text;
	}

	std::string read_rawstring(const std::string& filename)
	{
		std::string text, line;
		std::ifstream fin(filename);
		
		if (fin.is_open())
		{
			while (std::getline(fin, line))
			{
				if(line.empty())
				{
					continue;
				}
				text += line + '\n';
			}
			fin.close();
		}
		else
		{
			#ifdef HXUTILS_DEBUG
			std::cout << "FileUtil::read_file_as_string() : Failed to open '" + filename + "'" << std::endl;
			#endif
		}
		return text;
	}

	std::vector<std::vector<std::string>> read_csv(const std::string& filename)
	{
		std::vector<std::string> lines = read_file(filename);
		std::vector<std::vector<std::string>> tokens;

		for (std::string str : lines)
		{
			tokens.push_back(tokenize(str, ","));
		}
		return tokens;
	}

	void write_file(const std::string& filename, const std::string& buf)
	{
		std::ofstream file;
		file.open(filename);
		file << buf;
		file.close();
	}

	bool file_exists(const std::string& filepath)
	{
		std::ifstream file(filepath);
		return file.good();
	}
}