#pragma once

#include "indicator.h"
#include "candle.h"
#include "action.h"
#include <vector>

namespace daytrender
{
	class AlgorithmData
	{
	private:
		Indicator* _dataset = nullptr;
		short _capacity = 0;
		short _size = 0;
		short _action = 0;
		const char* _label = nullptr;
		const char* _error = nullptr;
		std::vector<int> _ranges;
		CandleSet _candles;


	public:
		inline void enter_long() { _action = Action::ENTER_LONG; }
		inline void exit_long() { _action = Action::EXIT_LONG; }
		inline void enter_short() { _action = Action::ENTER_SHORT; }
		inline void exit_short() { _action = Action::EXIT_SHORT; }

		AlgorithmData() = default;
		AlgorithmData(const std::vector<int>& ranges, const CandleSet& candles);
		AlgorithmData(const AlgorithmData& other);
		AlgorithmData(AlgorithmData&& other);
		~AlgorithmData();

		AlgorithmData& operator=(const AlgorithmData& other);
		inline Indicator& operator[](unsigned index) { return _dataset[index]; }
		inline const Indicator& operator[](unsigned index) const { return _dataset[index]; }

		inline int action() const { return _action; }
		inline short size() const { return _size; }
		inline short capacity() const { return _capacity; }
		inline void set_label(const char* label) { _label = label; }
		inline void flag_error(const char* error) { _error = error; }
		inline const char* label() const { return _label; }
		inline const char* error() const { return _error; }
		inline const CandleSet& candles() const { return _candles; }
		inline const std::vector<int>& ranges() const { return _ranges; }
		inline void increment_size() { _size++; }
	};
}
