#include "action.h"

#include <hirzel/fountain.h>
#include "../data/paperaccount.h"

namespace daytrender
{
	namespace action
	{
		bool nothing(TradeClient* client, double risk) { return true; }
		bool paper_nothing(PaperAccount& account, double risk) { return true; }

		bool buy(TradeClient* client, double risk) 
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

		bool sell(TradeClient* client, double risk)
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

		action_func actions[Action::COUNT] =
		{
			nothing,
			sell,
			buy
		};
		
		paper_func paper_actions[Action::COUNT] = 
		{
			paper_nothing,
			paper_sell,
			paper_buy
		};
	}
}