#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "symbols.h"

namespace dtbuild
{
	namespace lexer
	{
		extern std::unordered_map<std::string, short> token_types;

		struct token
		{
			std::string value;
			short type = NO_TYPE;
			long line = 0;
			int column = 0;
		};
		typedef std::vector<token> tokenlist;

		
		void init();
		tokenlist lex(const std::string& src, const std::string& filepath);
	}
}