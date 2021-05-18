#ifndef HIRZEL_RESULT_H
#define HIRZEL_RESULT_H

#include <utility>

namespace daytrender
{
	template <typename T>
	class Result
	{
	private:
		bool _ok = false;

		union
		{
			const char *error = nullptr;
			T *value;
		} _data;

	public:
		Result(T&& value)
		{
			_ok = true;
			_data.value = new T(value);
		}

		Result(Result&& other)
		{
			_ok = other._ok;
			_data = other._data;
			other._ok = false;
		}

		Result(const Result& other)
		{
			*this = other;
		}

		Result(const char *error)
		{
			_data.error = error;
		}

		~Result()
		{
			if (_ok) delete _data.value;
		}

		inline T&& get()
		{
			return std::move(*_data.value);
		}

		inline T value() { return _data.value; }
		inline const char* error() { return _data.error; }
		inline bool ok() { return _ok; }

		Result& operator=(const Result& other)
		{
			_ok = other.ok();
			if (_ok)
			{
				_data.value = other.get();
			}
			else
			{
				_data.error = other.error();
			}
		}

		inline operator bool() const { return _ok; }
	};
}

#endif
