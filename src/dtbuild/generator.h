#pragma once

#include "parser.h"

#include <string>

namespace dtbuild
{
	namespace generator
	{
		std::string generate(const parser::Node& tree);
	}	
}
