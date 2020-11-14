/*
	Copyright 2020 Ike Hirzel

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
	Brief: Intended not to be an improvement of std::any from C++17 but rather one
	that supports implicit conversions for ease of use and higher versatility. The primitive 
	data types { int, char, float, bool, etc.. } in general support implicit conversions so that
	the programmer does not have to take extra care to remember the exact typedef or type they
	put into the var. Type safety measures are also taken to be sure that data corruption and
	memory leaks do not occur
*/

/*
	Todo:
		* Fix as_str() conversions
		* add common arithmetic operators

*/

#include "var.h"

#include <string>
#include <cstdlib>

#define HIRZEL_VAR_INT_TYPE		0
#define HIRZEL_VAR_UINT_TYPE	1
#define HIRZEL_VAR_FLOAT_TYPE	2
#define HIRZEL_VAR_DOUBLE_TYPE	3
#define HIRZEL_VAR_CHAR_TYPE	4
#define HIRZEL_VAR_STR_TYPE		5
#define HIRZEL_VAR_BOOL_TYPE	6

namespace hirzel
{
	namespace details
	{
		// simple memcpy implementation to avoid including cstring
		void *write_bytes(void *to, const void *from, unsigned int size)
		{
			const char *src = (const char*)from;
			char *dst = (char*)to;
			for(unsigned int i = 0; i < size; i++)
			{
				dst[i] = src[i];
			}

			return dst;
		}

		// simple strlen implementation to avoid including cstring
		unsigned int cstr_len(const char* str)
		{
			unsigned int size = 0;
			const char*pos = str;
			while(*pos != 0)
			{
				size++;
				pos++;
			}
			return size;
		}
	}

	var::var(const void *_data, const std::type_info* _typeinfo, int _type, int _size)
	{
		primtype = _type;
		typeinfo = _typeinfo;
		size = _size;

		data = malloc(size);

		details::write_bytes(data, _data, size);
	}

	var::var() : var(nullptr, nullptr, -1, 0) {}
	var::var(const var& other) : var(other.as_bytes(), other.type(), other.primitive_type(), other.byte_count()) {}
	var::var(const void* _data, const std::type_info* _typeinfo, int _size) : var(_data, _typeinfo, -1, _size) {}

	var::var(short i) : var(&i, &typeid(short), HIRZEL_VAR_INT_TYPE, sizeof(short)) {}
	var::var(int i) : var(&i, &typeid(int), HIRZEL_VAR_INT_TYPE, sizeof(int)) {}
	var::var(long i) : var(&i, &typeid(long), HIRZEL_VAR_INT_TYPE, sizeof(long)) {}
	var::var(long long i) : var(&i, &typeid(long long), HIRZEL_VAR_INT_TYPE, sizeof(long long)) {}

	var::var(unsigned short i) : var(&i, &typeid(unsigned short), HIRZEL_VAR_UINT_TYPE, sizeof(unsigned short)) {}
	var::var(unsigned int i) : var(&i, &typeid(unsigned int), HIRZEL_VAR_UINT_TYPE, sizeof(unsigned int)) {}
	var::var(unsigned long i) : var(&i, &typeid(unsigned long), HIRZEL_VAR_UINT_TYPE, sizeof(unsigned long)) {}
	var::var(unsigned long long i) : var(&i, &typeid(unsigned long long), HIRZEL_VAR_UINT_TYPE, sizeof(unsigned long long)) {}

	var::var(char c) : var(&c, &typeid(char), HIRZEL_VAR_CHAR_TYPE, sizeof(char)) {}
	var::var(char *c) : var(c, &typeid(char*), HIRZEL_VAR_STR_TYPE, details::cstr_len(c) + 1) {}
	var::var(const char *c) : var(c, &typeid(const char*), HIRZEL_VAR_STR_TYPE, details::cstr_len(c) + 1) {}
	var::var(const std::string& s) : var(s.c_str(), &typeid(const char*), HIRZEL_VAR_STR_TYPE, s.size() + 1) {}

	var::var(float f) : var(&f, &typeid(float), HIRZEL_VAR_FLOAT_TYPE, sizeof(float)) {}
	var::var(double d) : var(&d, &typeid(double), HIRZEL_VAR_DOUBLE_TYPE, sizeof(double)) {}
	var::var(bool b) : var(&b, &typeid(bool), HIRZEL_VAR_BOOL_TYPE, sizeof(bool)) {}

	var::~var()
	{
		free(data);
		data = nullptr;
	}

	long long var::as_int() const
	{
		long long out = 0;
		switch(primtype)
		{
			case HIRZEL_VAR_INT_TYPE:
			case HIRZEL_VAR_UINT_TYPE:
			case HIRZEL_VAR_BOOL_TYPE:
			case HIRZEL_VAR_CHAR_TYPE:
				details::write_bytes(&out, data, size);
				break;

			case HIRZEL_VAR_FLOAT_TYPE:
				out = *(float*)data;
				break;

			case HIRZEL_VAR_DOUBLE_TYPE:
				out = *(double*)data;
				break;

			case HIRZEL_VAR_STR_TYPE:
			default:
				throw bad_var_cast();
				break;
		}
		return out;
	}

	unsigned long long var::as_uint() const
	{
		unsigned long long out = 0;
		switch(primtype)
		{
			case HIRZEL_VAR_INT_TYPE:
			case HIRZEL_VAR_UINT_TYPE:
			case HIRZEL_VAR_BOOL_TYPE:
			case HIRZEL_VAR_CHAR_TYPE:
				details::write_bytes(&out, data, size);
				break;

			case HIRZEL_VAR_FLOAT_TYPE:
				out = *(float*)data;
				break;

			case HIRZEL_VAR_DOUBLE_TYPE:
				out = *(double*)data;
				break;

			case HIRZEL_VAR_STR_TYPE:
			default:
				throw bad_var_cast();
				break;
		}

		return out;
	}

	float var::as_float() const
	{
		double out = 0;
		long long int i = 0;
		switch(primtype)
		{
			case HIRZEL_VAR_DOUBLE_TYPE:
				out = *(double*)data;
				break;
			
			case HIRZEL_VAR_FLOAT_TYPE:
				out = *(float*)data;
				break;

			case HIRZEL_VAR_INT_TYPE:
			case HIRZEL_VAR_UINT_TYPE:
			case HIRZEL_VAR_BOOL_TYPE:
			case HIRZEL_VAR_CHAR_TYPE:
				details::write_bytes(&i, data, size);
				out = i;
				break;

			case HIRZEL_VAR_STR_TYPE:
			default:
				throw bad_var_cast();
				break;
		}

		return out;
	}

	double var::as_double() const
	{
		double out = 0;
		long long i = 0;

		switch(primtype)
		{
			case HIRZEL_VAR_INT_TYPE:
			case HIRZEL_VAR_UINT_TYPE:
			case HIRZEL_VAR_BOOL_TYPE:
			case HIRZEL_VAR_CHAR_TYPE:
				details::write_bytes(&i, data, size);
				out = (double)i;
				break;

			case HIRZEL_VAR_DOUBLE_TYPE:
				return *(double*)data;
			
			case HIRZEL_VAR_FLOAT_TYPE:
				return *(float*)data;

			case HIRZEL_VAR_STR_TYPE:
			default:
				throw bad_var_cast();
				break;
		}
		return out;
	}

	char var::as_char() const
	{
		char out = 0;
		long long int i = 0;
		switch(primtype)
		{
			case HIRZEL_VAR_INT_TYPE:
			case HIRZEL_VAR_UINT_TYPE:
			case HIRZEL_VAR_BOOL_TYPE:
			case HIRZEL_VAR_CHAR_TYPE:
				details::write_bytes(&out, data, sizeof(char));
				break;

			case HIRZEL_VAR_FLOAT_TYPE:
				i =*(float*)data;
				out = i;
				break;

			case HIRZEL_VAR_DOUBLE_TYPE:
				i = *(double*)data;
				out = (char)i;
				break;

			case HIRZEL_VAR_STR_TYPE:
			default:
				throw bad_var_cast();
				break;
		}

		return out;
	}

	bool var::as_bool() const
	{
		bool out = false;
		long long int i = 0;
		switch(primtype)
		{
			case HIRZEL_VAR_INT_TYPE:
			case HIRZEL_VAR_UINT_TYPE:
			case HIRZEL_VAR_BOOL_TYPE:
			case HIRZEL_VAR_CHAR_TYPE:
				details::write_bytes(&out, data, sizeof(bool));
				break;

			case HIRZEL_VAR_FLOAT_TYPE:
				out = (bool)(*(float*)data);
				break;
			case HIRZEL_VAR_DOUBLE_TYPE:
				out = (bool)(*(double*)data);
				break;

			case HIRZEL_VAR_STR_TYPE:
			default:
				throw bad_var_cast();
		}
		return out;
	}

	std::string var::as_string() const
	{
		std::string out;
		long long i = 0;
		float f = 0;
		double d = 0;

		switch(primtype)
		{
			case HIRZEL_VAR_INT_TYPE:
			case HIRZEL_VAR_UINT_TYPE:
				details::write_bytes(&i, data, size);
				out = std::to_string(i);
				break;

			case HIRZEL_VAR_BOOL_TYPE:
				out = (*(bool*)data) ? "true" : "false";
				break;
				
			case HIRZEL_VAR_CHAR_TYPE:
				out += *(char*)data;
				break;

			case HIRZEL_VAR_FLOAT_TYPE:
				details::write_bytes(&f, data, size);
				out = std::to_string(f);
				break;

			case HIRZEL_VAR_DOUBLE_TYPE:
				details::write_bytes(&d, data, size);
				out = std::to_string(d);
				break;

			case HIRZEL_VAR_STR_TYPE:
				out = std::string((const char*)data);
				break;

			default:
				throw bad_var_cast();
		}
		return out;
	}

	const char* var::c_str() const
	{
		return as_string().c_str();
	}

	var& var::operator=(const var& other)
	{
		size = other.byte_count();
		free(data);
		data = malloc(size);
		details::write_bytes(data, other.as_bytes(), size);
		typeinfo = other.type();
		primtype = other.primitive_type();
		return *this;
	}
}
