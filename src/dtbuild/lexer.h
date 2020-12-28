#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "symbols.h"
#include <iostream>

#define CHAR_NOTYPE		0
#define CHAR_INVALID	1
#define CHAR_NAME		2
#define CHAR_SEP		3
#define CHAR_DIGIT		4
#define CHAR_OP			5
#define CHAR_LIT		6

namespace dtbuild 
{
	extern std::unordered_map<std::string, short> token_types;
	extern char char_types[256];

	struct Token
	{
		std::string filepath;
		std::string value;
		short type = NO_TYPE;
		long line = 0;
		short col = 0;
	};

	std::ostream& operator<<(std::ostream& out, const Token& t);

	void lex_init();
	std::vector<Token> lex(const std::string& filepath);
}