/**
 * @file	plugin.h
 * @brief	Provides declarations for Plugin and Function
 * @author	Ike Hirzel
 * 
 * Copyright 2020 Ike Hirzel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

namespace hirzel
{
	/**
	 * @brief	Typedef for basic void function pointer
	 */
	typedef void(*Function)();

	/**
	 * @brief	A Wrapper for binding to shared object plugins
	 * @details Binds to a shared object using OS's dylib and stores a map of functions
	 */
	class Plugin
	{
	private:
	// stores handle of library
	void* _lib = nullptr;
	// true if both lib and all functions are bound
	bool _bound = false;
	// error string
	const char* _error = nullptr;
	// filepath of the shared object
	std::string _filepath;
	// Stores pointers to the functions
	std::unordered_map<std::string, Function> _functions;

	public:
		/**
		 * @brief	Default constructor
		 */
		Plugin() = default;

		/**
		 * @brief	Binds library and functions with given names
		 * @param	filepath	path to dynamic library
		 * @param	funcnames	list of symbol names to bind (may be empty)
		 */
		Plugin(const std::string& filepath, const std::vector<std::string>& funcnames = {});

		/**
		 * @brief	Frees all bound symbols
		 */
		~Plugin();

		/**
		 * @brief	Binds dynamic library
		 * @param	filepath	path to dynamic library
		 */
		void bind_library(const std::string& filepath);
		
		/**
		 * @brief	Binds functions and adds pointer to it into map
		 * @param	funcname	symbolic name of function
		 * @return	pointer to function
		 */
		Function bind_function(const std::string& funcname);

		/**
		 * @brief	Calls function in the function map after casting it to type of templates.
		 * @tparam	\c T		cast for the return type of function
		 * @tparam	\c Args		casts for the parameters of function
		 * @param	funcname	symbol name of function
		 * @param	args		arguments to pass into function
		 */
		template <typename T, typename ...Args>
		T execute(const std::string& funcname, Args... args)
		{
			T(*func)(Args...) = (T(*)(Args...))_functions[funcname];
			// guard against function
			if(!func)
			{
				_error = "attempted to execute function that is not bound!";
				return T();
			}
			else
			{
				return func(args...);
			}
		}

		/**
		 * @return	true if library is bound, false if not
		 */
		inline bool is_lib_bound() const { return _lib; }

		/**
		 * @param	funcname	symbolic name of function
		 * @return	true if symbol is bound, false if it is not
		 */
		inline bool is_func_bound(const std::string& funcname) const
		{
			return _functions.find(funcname) != _functions.end();
		}

		/**
		 * @return true if library and all symbols bound correctly, false if not
		 */
		inline bool bound() const { return _bound; }

		/**
		 * @param	funcname	symbolic name of function
		 * @return	typeless pointer to function or nullptr if not bound
		 */
		inline Function get_func(const std::string& funcname)
		{
			return _functions[funcname];
		}

		/**
		 * @return	filepath of plugin
		 */
		inline const std::string& filepath() const { return _filepath; }

		/**
		 * @return	current error for plugin
		 */
		inline const char* error() const { return _error; }
	};
}

#endif // PLUGIN_H