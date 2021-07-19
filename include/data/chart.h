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
		short _size = 0;
		short _action = 0;
		const char* _label = nullptr;
		std::vector<unsigned> _ranges;
		PriceHistory _candles;

	public:
		Chart() = default;
		Chart(const std::vector<unsigned>& ranges, const PriceHistory& candles, unsigned window);
		Chart(const Chart& other);
		Chart(Chart&& other);

		~Chart();

		Chart& operator=(const Chart& other);
		inline Indicator& operator[](unsigned index) { return _dataset[index]; }
		inline const Indicator& operator[](unsigned index) const { return _dataset[index]; }

		inline void set_action(short action) { _action = action; }
		inline int action() const { return _action; }
		inline short size() const { return _size; }
		inline void set_label(const char* label) { _label = label; }
		inline const char* label() const { return _label; }
		inline const PriceHistory& candles() const { return _candles; }
		inline const std::vector<unsigned>& ranges() const { return _ranges; }
		inline void increment_size() { _size++; }
	};
}

#endif
