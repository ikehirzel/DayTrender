
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

#pragma once

#include <typeinfo>
#include <stdlib.h>
#include <string>

namespace hirzel
{
	struct bad_var_cast : public std::exception 
	{
		const char *what() const throw()
		{
			return "Invalid cast of hirzel::var";
		}
	};

	class var
	{
	private:
		// raw pointer to data
		void *data = nullptr;
		// type of data
		int primtype = -1;
		// size of data in bytes
		unsigned int size = 0;
		const std::type_info* typeinfo = nullptr;
		// constructor for primitives
		var(const void* _data, const std::type_info* _typeinfo, int type, int size);
		// constructor for other value types
		var(const void* _data, const std::type_info* _typeinfo, int _size);
	public:

		var();
		var(const var& other);
		template <typename T>
		var(const T& val) : var(&val, &typeid(T), sizeof(T)){}

		var(short i);
		var(int i);
		var(long i);
		var(long long i);

		var(unsigned short i);
		var(unsigned int i);
		var(unsigned long i);
		var(unsigned long long i);
		
		var(float f);
		var(double d);

		var(char c);
		var(char* c);
		var(const char* c);
		var(const std::string& s);

		var(bool b);

		~var();
		
		long long as_int() const;
		unsigned long long as_uint() const;
		float as_float() const;
		double as_double() const;
		char as_char() const;
		std::string as_string() const;
		const char* c_str() const;
		bool as_bool() const;

		inline const char *as_bytes() const
		{
			return (const char*)data;
		}

		inline unsigned int byte_count() const
		{
			return size;
		}

		inline const std::type_info* type() const
		{
			return typeinfo;
		}

		inline int primitive_type() const
		{
			return primtype;
		}

		template <typename T>
		T as()
		{
			T out;
			if(*typeinfo == typeid(T))
			{
				out = *(T*)data;
			}
			else
			{
				throw bad_var_cast();
			}
			return out;
		}


		var& operator=(const var& other);
	};
}
