#include "tradeclient.h"

namespace daytrender
{
	TradeClient::TradeClient(const std::vector<std::string>& credentials)
	{
		this->credentials = credentials;
	}
}