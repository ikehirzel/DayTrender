#pragma once

#include <string>

namespace dtbuild
{
	struct token;
	void syntax_error(const std::string& filepath, const std::string& msg, long line, int col, int width);
}