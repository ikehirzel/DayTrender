#include "indicator.h"

#include <iostream>


namespace daytrender
{
	Indicator::Indicator(int size)
	{
		_size = size;
		_data = new double[_size];
	}

	Indicator::Indicator(const Indicator& other)
	{
		*this = other;
	}

	Indicator::~Indicator()
	{
		delete[] _data;
	}

	Indicator& Indicator::operator=(const Indicator& other)
	{
		_size = other.size();
		_type = other.type();
		_label = other.label();
		_data = new double[_size];
		for (int i = 0; i < _size; i++)
		{
			_data[i] = other[i];
		}

		return *this;
	}
}