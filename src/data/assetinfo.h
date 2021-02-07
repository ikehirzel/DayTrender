#pragma once

namespace daytrender
{
	class AssetInfo
	{
	private:
		double _fee = 0.0;
		double _price = 0.0;
		double _shares = 0.0;

	public:
		AssetInfo() = default;
		AssetInfo(double fee, double price, double shares);

		inline double fee() const { return _fee; }
		inline double price() const { return _price; }
		inline double shares() const { return _shares; }
	};
}