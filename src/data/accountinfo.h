#pragma once

namespace daytrender
{
	class AccountInfo
	{
	private:
		double _balance = 0.0;
		double _bp_per_asset = 0.0;
		double _buying_power = 0.0;
		double _equity = 0.0;
		double _pl = 0.0;
		int _leverage = 0.0;
		bool _shorting_enabled = false;

	public:
		AccountInfo() = default;
		AccountInfo(double balance, double buying_power, double equity, int leverage, bool shorting_enabled);
		AccountInfo(const AccountInfo& other, double base_equity, double risk, int asset_count);

		inline double balance() const { return _balance; }
		inline double bp_per_asset() const { return _bp_per_asset; }
		inline double buying_power() const { return _buying_power; }
		inline double equity() const { return _equity; }
		inline double pl() const { return _pl; }
		inline int leverage() const { return _leverage; }
		inline bool shorting_enabled() const { return _shorting_enabled; }
	};
}
