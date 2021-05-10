#ifndef DAYTRENDER_CHART_H
#define DAYTRENDER_CHART_H

// local includes
#include <api/action.h>
#include <data/pricehistory.h>
#include <data/indicator.h>

// standard library
#include <vector>

namespace daytrender
{
	class Chart
	{
	private:
		Indicator* _dataset = nullptr;
		short _capacity = 0;
		short _size = 0;
		short _action = 0;
		const char* _label = nullptr;
		const char* _error = nullptr;
		std::vector<int> _ranges;
		PriceHistory _candles;

	public:
		inline void enter_long() { _action = Action::ENTER_LONG; }
		inline void exit_long() { _action = Action::EXIT_LONG; }
		inline void enter_short() { _action = Action::ENTER_SHORT; }
		inline void exit_short() { _action = Action::EXIT_SHORT; }

		Chart() = default;
		Chart(const std::vector<int>& ranges, const PriceHistory& candles, unsigned window);
		Chart(const Chart& other);
		Chart(Chart&& other);
		~Chart();

		Chart& operator=(const Chart& other);
		inline Indicator& operator[](unsigned index) { return _dataset[index]; }
		inline const Indicator& operator[](unsigned index) const { return _dataset[index]; }

		inline int action() const { return _action; }
		inline short size() const { return _size; }
		inline short capacity() const { return _capacity; }
		inline void set_label(const char* label) { _label = label; }
		inline void flag_error(const char* error) { _error = error; }
		inline const char* label() const { return _label; }
		inline const char* error() const { return _error; }
		inline const PriceHistory& candles() const { return _candles; }
		inline const std::vector<int>& ranges() const { return _ranges; }
		inline void increment_size() { _size++; }
	};
}

#endif
