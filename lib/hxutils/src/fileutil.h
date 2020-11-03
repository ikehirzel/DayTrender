#pragma once

#include <vector>
#include <string>
#include <cstdio>

namespace hirzel
{
	std::vector<std::string> read_file(const std::string& filename);
	std::string read_string(const std::string& filename);
	std::string read_rawstring(const std::string& filename);
	std::vector<std::vector<std::string>> read_csv(const std::string& filename);
	void write_file(const std::string& filename, const std::string& buf);
	bool file_exists(const std::string& filepath);
}
