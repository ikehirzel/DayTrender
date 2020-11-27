#pragma once

#include <vector>
#include <string>
#include <cstdio>

namespace hirzel
{
	namespace file
	{
		std::vector<std::string> read_file_as_vector(const std::string& filename);
		std::string read_file_as_string(const std::string& filename, bool ignore_empty_lines = false,
			const std::string& line_ending_fmt = "\n");
		std::vector<std::vector<std::string>> read_csv(const std::string& filename);
		void write_file(const std::string& filename, const std::string& buf);
		bool file_exists(const std::string& filepath);
	}
}
