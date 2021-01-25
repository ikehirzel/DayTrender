#include "action.h"

#include <hirzel/fountain.h>

namespace daytrender
{
	namespace action
	{
		bool nothing(Client* client, const std::string& ticker, double risk) { return true; }
		bool paper_nothing(PaperAccount& account, double risk) { return true; }

		
		bool paper_buy(PaperAccount& account, double risk)
		{
			double shares = (risk * account.buying_power() * (1.0 - account.fee())) / account.price();
			if (shares < account.minimum())
			{
				return false;
			}
			account.buy(shares);
			return true;
		}

		bool paper_sell(PaperAccount& account, double risk) 
		{
			double shares = account.shares();
			if (shares < account.minimum())
			{
				return false;
			}
			account.sell(shares);
			return true;
		}

		bool buy(Client* client, const std::string& ticker,  double risk) 
		{
			AccountInfo acct = client->get_account_info();
			double money_available = acct.money_per_share * risk;
			double price = client->get_price(ticker);
			double max_shares = money_available / price;
			double curr_shares = client->get_shares(ticker);
			double shares_to_order = max_shares - curr_shares;
			// if (||shares_to_order|| < client->minimum_order()) return false;
			return client->market_order(ticker, shares_to_order);
		}

		bool sell(Client* client, const std::string& ticker, double risk)
		{
			AccountInfo acct = client->get_account_info();
			double money_available = acct.money_per_share * risk;
			double curr_shares = client->get_shares(ticker);
			double price = client->get_price(ticker);
			double min_shares = 0.0;

			// if (client->shorting_enabled())
			// {
			// 	min_shares = -(money_available / price);
			// }

			double shares_to_order = -(curr_shares - min_shares);

			// if (||shares_to_order|| < client->minimum_order()) return false;

			return client->market_order(ticker, shares_to_order);
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