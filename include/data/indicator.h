#pragma once

namespace daytrender
{
	class Indicator
	{
	private:
		double* _data = nullptr;
		unsigned _size = 0;
		const char* _type = nullptr;
		const char* _label = nullptr;

	public:

		Indicator() = default;
		Indicator(unsigned size);
		Indicator(const Indicator& other);
		Indicator(Indicator&& other);
		~Indicator();
		Indicator& operator=(const Indicator& other);
		
		inline double& operator[](unsigned pos) { return _data[pos]; }
		inline double operator[](unsigned pos) const { return _data[pos]; }
		inline double back(unsigned pos = 0) const { return _data[(_size - 1) - pos]; }
		inline double front(unsigned pos = 0) const { return _data[pos]; }
		inline unsigned size() const { return _size; }
		inline void set_ident(const char* type, const char* label)
		{
			_type = type;
			_label = label;
		}
		inline const char* label() const { return _label; }
		inline const char* type() const { return _type; }
	};
}