#pragma once

#define ACTION_NOTHING	0U
#define ACTION_SELL		1U
#define ACTION_BUY		2U
#define ACTION_COUNT	3U

namespace daytrender
{
	class PaperAccount;
	class TradeClient;

	typedef bool (*action_func)(TradeClient*, double);
	typedef bool (*paper_func)(PaperAccount&, double);

	extern action_func actions[ACTION_COUNT];
	extern paper_func paper_actions[ACTION_COUNT];

	bool action_nothing(TradeClient* client, double risk);
	bool paper_nothing(PaperAccount& account, double risk);

	bool action_buy(TradeClient* client, double risk);
	bool paper_buy(PaperAccount& account, double risk);

	bool action_sell(TradeClient* client, double risk);
	bool paper_sell(PaperAccount& account, double risk);

};