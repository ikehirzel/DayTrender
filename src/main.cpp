#include "daytrender.h"

#include <iostream>
#include <hirzel/fountain.h>

using namespace daytrender;

int main()
{
	hirzel::log_init("./report.log");
	DayTrender dt;
	dt.start();
	return 0;
}