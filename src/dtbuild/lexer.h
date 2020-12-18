#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "symbols.h"
#include <iostream>

namespace dtbuild 
{
	namespace lexer
	{
		extern std::unordered_map<std::string, short> token_types;

		struct Token
		{
			std::string value;
			short type = NO_TYPE;
			long line = 0;
			int col = 0;
		};

		std::ostream& operator<<(std::ostream& out, const Token& t);

		void init();
		std::vector<Token> lex(const std::string& src, const std::string& filepath);
	}
}