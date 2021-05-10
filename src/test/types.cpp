// local includes
#include <data/pricehistory.h>
#include <data/indicator.h>
#include <data/chart.h>
#include <data/position.h>
#include <data/account.h>

// standard library
#include <assert.h>
#include <stdio.h>

#include <string>

using namespace daytrender;

int main(void)
{
	// assuring that all the types are c memory layout compliant
	// for clients
	assert(std::is_standard_layout<Candle>());
	assert(std::is_standard_layout<PriceHistory>());
	assert(std::is_standard_layout<Account>());
	assert(std::is_standard_layout<Position>());
	assert(std::is_standard_layout<Indicator>());
	assert(std::is_standard_layout<Chart>());
	puts("All types are Standard Layout");
	return 0;
}