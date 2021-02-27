/**
 * @file	plugin.cpp
 * @brief	Provides definitions for Plugin
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
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "plugin.h"

#if defined(_WIN32) || defined(_WIN64)

#define OS_IS_WINDOWS true
#include <windows.h>

#elif defined(__unix__) || defined(linux)

#define OS_IS_WINDOWS false
#include <dlfcn.h>

#endif

namespace hirzel
{
	Plugin::Plugin(const std::string& filepath, const std::vector<std::string>& funcnames)
	{
		bind_library(filepath);

		if(_lib)
		{
			for(const std::string& s : funcnames)
			{
				bind_function(s);
			}
		}
	}

	Plugin::~Plugin()
	{
		if(_lib)
		{
			#if OS_IS_WINDOWS
			FreeLibrary((HINSTANCE)_lib);
			#else
			dlclose(_lib);
			#endif
		}
	}

	void Plugin::bind_library(const std::string& filepath)
	{
		if(_lib)
		{
			_error = "a library is already bound! overwriting is not allowed.";
			return;
		}

		_filepath = filepath;

		#if OS_IS_WINDOWS
		_lib = (void*)LoadLibrary(_filepath.c_str());
		#else
		_lib = dlopen(_filepath.c_str(), RTLD_NOW);
		#endif

		if(!_lib)
		{
			#if OS_IS_WINDOWS
			_error = GetLastError();
			#else
			_error = dlerror();
			#endif

			_bound = false;
		}
		else
		{
			_bound = true;
		}
	}

	Function Plugin::bind_function(const std::string& funcname)
	{
		// function pointer that will be stored
		Function func;

		//guard against unloaded library
		if(!_lib)
		{
			_error = "lib has not been bound! cannot continue with binding function!";
			return nullptr;
		}

		//loading function from library
		#if OS_IS_WINDOWS
		func = (func_ptr)GetProcAddress((HINSTANCE)_lib, funcname.c_str());
		#else
		func = (Function)dlsym(_lib, funcname.c_str());
		#endif

		// guard against unbound function
		if(!func)
		{
			#if OS_IS_WINDOWS
			_error = GetLastError();
			#else
			_error = dlerror();
			#endif
			
			_bound = false;
			return nullptr;
		}
		else
		{
			// putting function into map
			_functions[funcname] = func;
			return func;
		}
	}
}