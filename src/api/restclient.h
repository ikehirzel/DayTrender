#pragma once

#include "../data/candle.h"

#include <vector>
#include <nlohmann/json.hpp>
#include <hlog.h>

namespace httplib
{
	class SSLClient;
}

using json = nlohmann::json;

namespace daytrender
{
	#define METHOD_GET		0U
	#define METHOD_POST		1U
	#define METHOD_PATCH	2U

	typedef std::pair<std::string, std::string> arg;
	typedef std::vector<arg> arglist;

	

	class RestClient
	{
	public:
		RestClient(const std::string& base_url, const arglist& headers = {});
		~RestClient();
		static const char *METHOD_NAMES[3];
	protected:
		httplib::SSLClient* client;
		std::string base_url;
		arglist headers;
		hirzel::Logger l;
	public:
		void set_bearer_token(const std::string& bearer);
		json jsonbody_request(unsigned int method, const std::string& endpoint,
			const json& params);
			
		json urlquery_request(unsigned int method, const std::string& endpoint,
			const arglist& params);
		json get(const std::string& endpoint, const arglist& params = {});
		json post(const std::string& endpoint, const arglist& params = {});
		json post(const std::string& endpoint, const json& params = nullptr);

	};
}