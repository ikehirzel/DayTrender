// local includes
#include <data/tradesystem.h>

// standard library
#include <cstring>
#include <csignal>
#include <filesystem>

// external libraries
#include <hirzel/logger.h>
#include <hirzel/util/str.h>

#define COLOR_RED		"\033[31m"
#define COLOR_RESET		"\033[0m"
#define ERROR_PROMPT	COLOR_RED "error: " COLOR_RESET

const char * const HELP_PROMPT = R"=====(available commands:

    account
    backtest
    price
)=====";

using namespace daytrender;

bool portfolio_error()
{
	PRINT(ERROR_PROMPT "portfolio requested does not exist\n");
	return false;
}

bool cli_account(TradeSystem& system, int argc, const char *args[], const char *dir)
{
	if (argc != 1)
	{
		PRINT(ERROR_PROMPT "account takes in portfolio label as argument\n");
		return false;
	}

	const char *label = args[0];

	Portfolio *portfolio = system.get_portfolio(label);
	if (!portfolio) return portfolio_error();

	Client& cli = portfolio->get_client();

	Result<Account> res = cli.get_account();
	if (!res)
	{
		PRINT(ERROR_PROMPT "%s", res.error());
		return false;
	}

	Account acc = res.get();

	PRINT("%s Account:\n\tBalance:       %f\n\tBuying Power:  %f\n\tMargin Used:"
		"   %f\n\tEquity:        %f\n\tLeverage:      %u:1\n\tShorting:      %t\n",
		label, acc.balance(), acc.buying_power(), acc.margin_used(),
		acc.equity(), acc.leverage(), acc.shorting_enabled());

	return true;
}

bool cli_backtest(TradeSystem& system, int argc, const char *args[], const char *dir)
{
	return false;
}

bool cli_price(TradeSystem& system, int argc, const char *args[], const char *dir)
{
	if (argc != 4)
	{
		PRINT(ERROR_PROMPT "args must be as follows:\n\tportfolio label\n\tasset ticker\n\tcandle interval\n\tcandle count\n");
		return false;
	}
	// need to get ticker, client, count
	const char *client = args[0];
	const char *ticker = args[1];
	const char *interval = args[2];
	const char *count = args[3];

	int count_num = std::stoi(count);
	int interval_num = std::stoi(interval);

	Portfolio *portfolio = system.get_portfolio(client);
	if (!portfolio) return portfolio_error();

	Client& cli = portfolio->get_client();

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

bool handle_input(TradeSystem& system, int argc, const char *args[], const char *dir)
{
	switch (args[0][0])
	{
	case 'a':
		if (!std::strcmp(args[0], "account"))
			return cli_account(system, argc - 1, args + 1, dir);
	case 'b':
		if (!std::strcmp(args[0], "backtest"))
			return cli_backtest(system, argc - 1, args + 1, dir);
		break;

	case 'p':
		if (!std::strcmp(args[0], "price"))
			return cli_price(system, argc - 1, args + 1, dir);
		break;

	case 'h':
		if (!std::strcmp(args[0], "help"))
		{
			PRINT(HELP_PROMPT);
			return true;
		}
	}

	PRINT(ERROR_PROMPT "invalid command\n\n%s\n", HELP_PROMPT);
	return false;
}

daytrender::TradeSystem *system_ref = nullptr;

void interrupt(int signal)
{
	if (!system_ref) exit(signal);
	system_ref->stop();
}

int main(int argc, const char *argv[])
{
	bool command_line = argc > 1;
	// if using as cli, do not print normal logs
	hirzel::logger::init(true, !command_line, "", 0UL);

	std::string dir = std::filesystem::current_path().string() + "/" + hirzel::str::get_folder(argv[0]);

	// cut off '.' part of directory if it is there
	if (dir.back() == '.') dir.resize(dir.size() - 2);

	// initializing trade system
	daytrender::TradeSystem system(dir);
	system_ref = &system;

	if (!system.is_initialized())
	{
		FATAL("DayTrender failed to initialize");
		if (command_line) PRINT("DayTrender failed to initialize. Run in trade mode for more information\n");
		return 1;
	}

	// handling console input
	if (command_line) return !handle_input(system, argc - 1, argv + 1, dir.c_str());

	// setting up handler for keyboard interrupts
	std::signal(SIGINT, interrupt);

	// returns when program has ended
	system.start();

	SUCCESS("DayTrender has stopped");
	return 0;
}