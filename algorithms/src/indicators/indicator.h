#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "../../src/data/candle.h"

typedef std::pair<std::string, std::vector<double>> indicator_data;
typedef std::map<std::string, std::vector<double>> indicator_dataset;

class Indicator
{
protected:
	std::string name;
	unsigned int denom = 1;
	Indicator(const std::string& name, unsigned int denom)
	{
		this->name = name;
		this->denom = denom;
	}

public:
	virtual ~Indicator() = default;
	virtual indicator_data calculate(const daytrender::candleset& candles, unsigned int index,
		unsigned int window) = 0;
	inline std::string getName() const { return name; }
};
