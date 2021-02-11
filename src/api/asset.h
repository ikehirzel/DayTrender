#pragma once

#include <string>
#include <vector>

#include "../data/candle.h"
#include "algorithm.h"
#include "client.h"

namespace daytrender
{
	class Asset
	{
	private:
		mutable bool _live = false;
		int _candle_count = 0;
		int _interval = 0;
		int _type = -1;
		long long _last_update = 0;
		double _risk = 0.0;

		std::string _ticker;
		AlgorithmData _data;
		std::vector<int> _ranges;
		
		Client* _client;
		const Algorithm* _algo;
		typedef bool (Asset::*AssetAction)();
		AssetAction _actions[Action::COUNT];
		
		// client wrappers
		inline CandleSet get_candles() const
		{
			return _client->get_candles(_ticker, _interval, _candle_count, _algo->data_length());
		}

	public:
		Asset(Client* client, const Algorithm* algo, const std::string& ticker, int type,
			int interval, const std::vector<int>& ranges);

		void update();

		// inline getter functions
		inline AlgorithmData data() const { return _data; }
		inline const Algorithm* algorithm() const { return _algo; }
		inline const Client* client() const { return _client; }
		inline const std::string& ticker() const { return _ticker; }
		inline int interval() const { return _interval; }
		inline int type() const { return _type; }
		inline bool is_live() const { return _live && _client->is_live(); }
		inline double risk() const { return _risk; }
		inline int data_length() const { return _algo->data_length(); }
	};
}