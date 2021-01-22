#include "action.h"

#include <hirzel/fountain.h>
#include "../data/paperaccount.h"

namespace daytrender
{
	namespace action
	{
		bool nothing(const Client* client, const std::string& ticker, double risk) { return true; }
		bool paper_nothing(PaperAccount& account, double risk) { return true; }

		bool buy(const Client* client, const std::string& ticker,  double risk) 
		{
			//double curr_shares = client->get_shares(t
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

		bool sell(const Client* client, const std::string& ticker, double risk)
		{
			if (risk > 1.0) risk = 1.0;
			AccountInfo info = client->get_account_info();
			double curr_shares = client->get_shares(ticker);
			double price = client->get_price(ticker);
			// check if the amount of shares controlled by the asset 
			// oversteps the amount available to it and act skip if so
			double curr_value = curr_shares * price;
			// setting to absolute value of the shares
			curr_value  = (curr_value >= 0 ? curr_value : -curr_value);
			double percent = client->get_asset_share() * risk;
			double money_available = curr_value - percent * info.equity;
			double shares = 0.0;
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

		ActionFunc actions[Action::COUNT] =
		{
			nothing,
			sell,
			buy
		};
		
		PaperFunc paper_actions[Action::COUNT] = 
		{
			paper_nothing,
			paper_sell,
			paper_buy
		};
	}
}