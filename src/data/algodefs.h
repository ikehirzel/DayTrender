#pragma once

#include <vector>
#include <unordered_map>

namespace daytrender
{
	struct indicator_data
	{
		std::string type, label;
		std::vector<double> data;
	};
	
	struct algorithm_data
	{
		std::vector<indicator_data> dataset;
		int action;
	};
}
