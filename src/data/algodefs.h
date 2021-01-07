#include "algotypes.h"

using namespace daytrender;

typedef std::vector<double> indicator;

extern "C" int arg_count();
extern "C" bool algorithm (algorithm_data& out);

const indicator& add_indicator(algorithm_data& out, int pos, indicator (*indi)(const candleset&, int range), std::string type, std::string label)
{
	out.dataset[pos].data = indi(out.candles, out.ranges[pos + 1]);
	out.dataset[pos].type = type;
	out.dataset[pos].label = label;
	return out.dataset[pos].data;
}

bool init_algorithm(algorithm_data& out, const std::string& label)
{
	bool success = true;
	out.label = label;
	int size = arg_count() - 1;
	out.dataset.resize(size);

	if (out.candles.empty())
	{
		out.err += "No candles were passed to algorithm! ";
		success = false;
	}

	if (size < out.ranges.size() - 1)
	{
		out.err += "Too many ranges were passed into algorithm. ";
		success = false;
	}
	else if (size >= out.ranges.size())
	{
		out.err += "Not enough ranges were passed into algorithm. ";
		success = false;
	}

	if (out.ranges.size() <= size)
	{
		std::string msg = "Not enough ranges were passed into algorithm: expected ";
		msg += std::to_string(size + 1);
		msg += ", received " + std::to_string(out.ranges.size());
		msg += "! ";
		out.err += msg;
		success = false;
	}
	else if (out.ranges.size() > size + 1)
	{
		std::string msg = "Too many ranges were passed into algorithm: expected ";
		msg += std::to_string(size + 1);
		msg += ", received " + std::to_string(out.ranges.size());
		msg += "! ";
		out.err += msg;
		success = false;
	}
	return success;
}
