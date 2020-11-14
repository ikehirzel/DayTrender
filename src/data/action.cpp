#include "action.h"

#include <hirzel/fountain.h>
#include "paperaccount.h"

namespace daytrender
{
	action_func actions[ACTION_COUNT] = { &action_nothing, &action_sell, &action_buy };
	paper_func paper_actions[ACTION_COUNT] = { &paper_nothing, &paper_sell, &paper_buy };

	bool action_nothing(TradeClient* client, double risk) { return true; }
	bool paper_nothing(PaperAccount& account, double risk) { return true; }

	bool action_buy(TradeClient* client, double risk) 
	{
		errorf("Live buying is not implemented yet!");
		return false;
	}

	bool paper_buy(PaperAccount& account, double risk)
	{
		double shares = (risk * account.getBalance() * (1.0 - account.getFee())) / account.getPrice();
		if (shares < account.getMinimum())
		{
			return false;
		}
		account.buy(shares);
		return true;
	}

	bool action_sell(TradeClient* client, double risk)
	{
		errorf("Live selling is not implemented yet!");
		return false;
	}

	bool paper_sell(PaperAccount& account, double risk) 
	{
		double shares = account.getShares();
		if (shares < account.getMinimum())
		{
			return false;
		}
		account.sell(shares);
		return true;
	}
}