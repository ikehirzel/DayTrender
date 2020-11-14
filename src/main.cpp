#include "daytrender.h"

#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

int main(int argc, char *argv[])
{
	hirzel::fountain::init("./report.log", true);
	daytrender::init(hirzel::get_folder(argv[0]));
	daytrender::start();
	infof("DayTrender has stopped");
	return 0;
}