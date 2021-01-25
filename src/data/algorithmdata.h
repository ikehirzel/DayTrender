#pragma once

#include "indicator.h"
#include "candle.h"
#include "actionenum.h"
#include <vector>

namespace daytrender
{
	class AlgorithmData
	{
	private:
		Indicator* _dataset = nullptr;
		int _size = 0;
		int _action = 0;
		const char* _label = nullptr;
		const char* _error = nullptr;
		std::vector<int> _ranges;
		CandleSet _candles;

	public:
		inline void do_nothing() { _action = Action::NOTHING; }
		inline void sell() { _action = Action::SELL; }
		inline void buy() { _action = Action::BUY; }

		AlgorithmData() = default;
		AlgorithmData(const std::vector<int>& ranges, const CandleSet& candles);
		AlgorithmData(const AlgorithmData& other);
		~AlgorithmData();

		AlgorithmData& operator=(const AlgorithmData& other);
		inline Indicator& operator[](unsigned index) { return _dataset[index]; }
		inline const Indicator& operator[](unsigned index) const { return _dataset[index]; }

		inline int action() const { return _action; }
		inline int size() const { return _size; }
		inline void set_label(const char* label) { _label = label; }
		inline void flag_error(const char* error) { _error = error; }
		inline const char* label() const { return _label; }
		inline const char* error() const { return _error; }
		inline const CandleSet& candles() const { return _candles; }
		inline const std::vector<int>& ranges() const { return _ranges; }
	};
}
