#include "action.h"

#include <hirzel/fountain.h>
#include "paperaccount.h"

namespace daytrender
{
	action_func actions[ACTION_COUNT] = { &action_nothing, &action_sell, &action_buy };
	paper_func paper_actions[ACTION_COUNT] = { &paper_nothing, &paper_sell, &paper_buy };

	void action_nothing(TradeClient* client, double risk) {}
	void paper_nothing(PaperAccount* account, double risk) {}

	void action_buy(TradeClient* client, double risk) 
	{
		errorf("Live buying is not implemented yet!");
	}

	void paper_buy(PaperAccount* account, double risk)
	{
		if (!account)
		{
			errorf("Paper account supplied was nullptr!");
			return;
		}

		double shares = (risk * account->getBalance() * (1.0 - account->getFee())) / account->getPrice();

		if (shares < account->getMinimum())
		{
			warningf("Shares requested was below minimum, skipping purchase");
			return;
		}

		account->buy(shares);
	}

	void action_sell(TradeClient* client, double risk)
	{
		errorf("Live selling is not implemented yet!");
	}

	void paper_sell(PaperAccount* account, double risk) 
	{
		if (!account)
		{
			errorf("Paper account supplied was nullptr!");
			return;
		}

		double shares = account->getShares();

		if (shares < account->getMinimum())
		{
			warningf("Shares requested was below minimum, skipping sale");
			return;
		}
		account->sell(shares);
	}
}