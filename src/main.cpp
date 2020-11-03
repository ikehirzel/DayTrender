#include "daytrender.h"

#include <iostream>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>
using namespace daytrender;

int main(int argc, char *argv[])
{
	hirzel::log_init("./report.log");

	DayTrender dt(hirzel::get_folder(argv[0]));
	dt.start();
	return 0;
}