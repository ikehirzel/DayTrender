#include "restclient.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <iostream>

using namespace httplib;

#define RESTCLIENT_NAME "RestClient"

namespace daytrender
{
	const char* RestClient::METHOD_NAMES[3] = {
		"GET",
		"POST",
		"PATCH"
	};

	RestClient::RestClient(const std::string& base_url, const arglist& headers)
	{
		l = hirzel::Logger(RESTCLIENT_NAME);
		this->base_url = base_url;
		client = new SSLClient(base_url);
		this->headers = headers;
		Headers default_headers;
		for(arg h : headers)
		{
			default_headers.insert(h);
		}
		client->set_default_headers(default_headers);
	}

	RestClient::~RestClient()
	{
		delete client;
	}

	void RestClient::set_bearer_token(const std::string& bearer)
	{
		client->set_bearer_token_auth(bearer.c_str());
	}

	json RestClient::jsonbody_request(unsigned int method, const std::string& endpoint,
			const json& params)
	{
		std::string m = METHOD_NAMES[method];

		Params p;
		std::string body = params.dump();

		Result res{nullptr, Success};

		switch(method)
		{
			case METHOD_GET:
					l.error(m + " is not allowed for this operation!");
					return 0;
				break;

			case METHOD_POST:
				res = client->Post(endpoint.c_str(), body, "application/json");
				break;

			case METHOD_PATCH:
				res = client->Patch(endpoint.c_str(), body, "application/json");
				break;
		}

		json data;

		if(!res)
		{
			l.error(m + " failed to get reponse!");
			std::cout << "RestClient::jsonbody_request(" << METHOD_NAMES[method] << ") : Failed to get reponse!\n";
			return data;
		}

		if(res->status != 200)
		{
			l.warning(m + " status was not OK: " + std::to_string(res->status));
			std::cout << "RestClient::jsonbody_request(" << METHOD_NAMES[method] << ") : Status was not OK : STATUS_CODE "
				<< res->status << std::endl;
		}

		if(res->body.empty())
		{
			l.error(m + " body was empty!");
			std::cout << "RestClient::jsonbody_request(" << METHOD_NAMES[method] << ") : Response body was empty!\n";
			return data;
		}

		data = json::parse(res->body);
		return data;
	}

	json RestClient::urlquery_request(unsigned int method, const std::string& endpoint, const arglist& params)
	{	
		std::string m = METHOD_NAMES[method];
		Params p;

		for(arg a : params)
		{
			p.insert(a);
		}

		std::string url = endpoint;
		std::string query = detail::params_to_query_str(p);
		
		if(!params.empty())
		{
			url += ("?" + query);
		}

		l.info(m + " @ " + base_url + url);
		Result res{nullptr, Success};

		switch(method)
		{
			case METHOD_GET:
				res = client->Get(url.c_str());
				break;

			case METHOD_POST:
				res = client->Post(endpoint.c_str(), p);
				break;
		}

		json data;
		if(!res)
		{
			l.error(m + " failed to get reponse!");
			return data;
		}

		if(res->status != 200)
		{
			l.warning(m + " status was not OK: " + std::to_string(res->status));
		}

		if(res->body.empty())
		{
			l.error(m + " body was empty!");
			return data;
		}

		data = json::parse(res->body);
		return data;
	}

	json RestClient::get(const std::string& endpoint, const arglist& params)
	{
		return urlquery_request(METHOD_GET, endpoint, params);
	}


	json RestClient::post(const std::string& endpoint, const arglist& params)
	{
		return urlquery_request(METHOD_POST, endpoint, params);
	}

	json RestClient::post(const std::string& endpoint, const json& params)
	{
		return jsonbody_request(METHOD_POST, endpoint, params);
	}
}