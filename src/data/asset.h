#pragma once

#include <string>
#include <vector>

#include "candle.h"
#include "../api/paperaccount.h"
#include "../api/algorithm.h"
#include "../api/client.h"

// algorithm constants
#define MAX_ALGORITHM_WINDOW	50
#define MIN_ALGORITHM_WINDOW	4


#define PAPER_ACCOUNT_INITIAL 500.0

namespace daytrender
{
	class Asset
	{
	private:
		bool _paper = true;
		bool _live = false;

		int _interval = 0;
		int _candle_count = 0;
		int _type = 0;

		long long _last_update = 0;

		double _risk = 0.9;

		std::string _ticker;
		AlgorithmData _data;
		std::vector<int> _ranges;
		
		Client* _client;
		const Algorithm* _algo;
		PaperAccount _paper_account;
		
	public:
		Asset(int type, Client* client, const std::string &ticker, const Algorithm* algo,
			int interval, double risk, const std::vector<int>& ranges, bool paper);

		void update();
		//AssetInfo info() const;
		bool should_update() const;

		// inline getter functions
		inline AlgorithmData data() const { return _data; }
		inline const Algorithm* algorithm() const { return _algo; }
		inline Client* client() const { return _client; }
		inline const std::string& ticker() const { return _ticker; }
		inline int interval() const { return _interval; }
		inline int type() const { return _type; }
		inline bool is_live() const { return _live && _client->is_live(); }
		inline int risk() const { return _risk; }
		inline bool is_paper() const { return _paper; }
	};
}