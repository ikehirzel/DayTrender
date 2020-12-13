#pragma once

#include "parser.h"

#include <string>

namespace dtbuild
{
	namespace generator
	{
		std::string generate_code(const parser::Node& tree);
	}	
}
