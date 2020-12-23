#pragma once

#include "lexer.h"

#include <vector>
#include <unordered_map>
#include <iostream>


namespace dtbuild
{
	namespace parser
	{
		extern std::vector<std::string> ntnames;

		struct Node
		{
			short type = NO_TYPE;  
			short subtype = NO_TYPE;
			std::string value;
			std::vector<Node> args;

			void print(int depth = 0)
			{
				for (int i = 0; i < depth; i++) std::cout << "|\t";
				std::cout << ntnames[type];

				if (!value.empty())
				{
					std::cout << ": '" << value << "'";
				}
				std::cout << "\n";

				for (int j = 0; j < args.size(); j++)
				{
					args[j].print(depth + 1);
				}
			}

			bool empty() const
			{
				return value.empty() && args.empty();
			}
		};

		void init();
		Node parse(const std::vector<lexer::Token>& toks, const std::string& filepath);
	}
}