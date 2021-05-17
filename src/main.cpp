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
	if (argc < 4)
	{
		PRINT("daytrender: not enough args supplied!\nThey must be in the following order:\n\tclient filename\n\tasset ticker\n\tcandle interval\n\tcandle count\n");
		return false;
	}
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
		ERROR("%s: %s", client, res.error());
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

	PRINT("daytrender: Invalid command\n");
	return false;
}

int main(int argc, const char *argv[])
{
	bool command_line = argc > 1;
	// if using as cli, do not print normal logs
	hirzel::logger::init(true, !command_line, "", 0UL);

	std::string dir = std::filesystem::current_path().string() + "/" + hirzel::str::get_folder(argv[0]);


	// initializing trade system
	daytrender::TradeSystem system(hirzel::str::get_folder(argv[0]));

	if (!system.is_initialized())
	{
		FATAL("DayTrender failed to initialize");
		if (command_line) PRINT("DayTrender failed to initialize. Run in trade mode for more information\n");
		return 1;
	}

	// handling console input
	if (command_line) return !handle_input(argc - 1, argv + 1, dir.c_str());

	// returns when program has ended
	system.start();

	SUCCESS("DayTrender has stopped");
	return 0;
}