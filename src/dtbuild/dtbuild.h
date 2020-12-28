#pragma once

#include <string>

namespace dtbuild
{
	extern std::string cwd;
	extern std::string execdir;

	std::string syntax_error(const std::string& filepath, const std::string& msg, long line, int col, int width);
	std::string tabs(int amt);
}