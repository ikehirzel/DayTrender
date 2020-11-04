#pragma once

#define ACTION_NOTHING	0U
#define ACTION_SELL		1U
#define ACTION_BUY		2U
#define ACTION_COUNT	3U
namespace daytrender
{
	class PaperAccount;
	class TradeClient;

	typedef void (*action_func)(TradeClient*, double);
	typedef void (*paper_func)(PaperAccount*, double);

	extern action_func actions[ACTION_COUNT];
	extern paper_func paper_actions[ACTION_COUNT];

	void action_nothing(TradeClient* client, double risk);
	void paper_nothing(PaperAccount* account, double risk);

	void action_buy(TradeClient* client, double risk);
	void paper_buy(PaperAccount* account, double risk);

	void action_sell(TradeClient* client, double risk);
	void paper_sell(PaperAccount* account, double risk);

};