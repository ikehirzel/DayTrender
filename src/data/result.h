#ifndef HIRZEL_RESULT_H
#define HIRZEL_RESULT_H

namespace daytrender
{
	template <typename T>
	class Result
	{
	private:
		T _value;
		const char *_error = nullptr;

	public:
		inline Result(T value)
		{
			_value = value;
		}

		inline Result(const char *error)
		{
			_error = error;
		}

		inline T get()
		{
			if (!_error) return _value;
			return {};
		}

		inline const char* error() { return _error; }
		inline bool ok() { return !_error; }
	};

	template <typename T>
	class Status
	{
	private:
		const char* _error = nullptr;
	public:
		Status() = default;
		Status(const char *error);
		inline const char *error() { return _error; }
	};
}

#endif
