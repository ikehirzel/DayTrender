// local includes
#include <data/tradesystem.h>

// standard library
#include <cstring>
#include <filesystem>

// external libraries
#include <hirzel/logger.h>
#include <hirzel/util/str.h>


using namespace daytrender;

bool cli_backtest(int argc, const char *args[], const char *dir)
{
	if (!argc) return false;
	const char *strat = nullptr;

	return false;
}

bool cli_price(int argc, const char *args[], const char *dir)
{
	if (argc < 4) return false;
	// need to get ticker, client, count
	const char *client = args[0];
	const char *ticker = args[1];
	const char *interval = args[2];
	const char *count = args[3];

	int count_num = std::stoi(count);
	int interval_num = std::stoi(interval);

	Client cli(client, dir);

	Result<PriceHistory> res = cli.get_price_history(ticker, interval_num, count_num);
	if (!res.ok())
	{
		ERROR(res.error());
		return false;
	}

	PriceHistory ph = res.get();
	for (size_t i = 0; i < ph.size(); ++i)
	{
		PRINT("O: %f, H: %f, L: %f, C: %f, V: %f\n", ph[i].o(), ph[i].h(), ph[i].l(), ph[i].c(), ph[i].v());
	}


	return true;
}

bool handle_input(int argc, const char *args[], const char *dir)
{
	switch (args[0][0])
	{
	case 'b':
		if (!std::strcmp(args[0], "backtest"))
			return cli_backtest(argc - 1, args + 1, dir);
		break;

	case 'p':
		if (!std::strcmp(args[0], "price"))
			return cli_price(argc - 1, args + 1, dir);
		break;
	}

	PRINT("Invalid command\n");
	return false;
}

int main(int argc, const char *argv[])
{
	hirzel::logger::init();
	std::string dir = std::filesystem::current_path().string() + "/" + hirzel::str::get_folder(argv[0]);

	// handling console input
	if (argc > 1)
	{
		if (handle_input(argc - 1, argv + 1, dir.c_str())) return 0;
		return 1;
	}


	// trade system
	daytrender::TradeSystem system(hirzel::str::get_folder(argv[0]));
	return 0;
	if (!system.is_initialized())
	{
		FATAL("DayTrender failed to initialize");
		return 1;
	}

	system.start();
	
	SUCCESS("DayTrender has stopped");
	return 0;
}