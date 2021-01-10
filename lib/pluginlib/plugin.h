/*
	plugin.h
	Copyright 2020 Ike Hirzel
	MIT License
*/

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)

#define OS_IS_WINDOWS true
#include <windows.h>

#elif defined(__unix__) || defined(linux)

#define OS_IS_WINDOWS false
#include <dlfcn.h>

#endif

namespace hirzel
{
	typedef void(*func_ptr)();
	class Plugin
	{
	private:
	// Stored handle of library
	void *lib = nullptr;
	// Stores pointers to the functions
	std::string filename, err_glob;
	std::unordered_map<std::string, func_ptr> functions;

	public:
		Plugin() = default;
		Plugin(const std::string& _filename, const std::vector<std::string>& funcnames = {})
		{
			filename = _filename;
			bind_library(filename);

			if(lib)
			{
				for(const std::string& s : funcnames)
				{
					bind_function(s);
				}
			}
			else
			{
				err_glob += filename + ": binding of functions could not continue!\n";
			}
		}

		// Frees loaded handle
		~Plugin()
		{
			if(lib)
			{
				#if OS_IS_WINDOWS
				FreeLibrary((HINSTANCE)lib);
				#else
				dlclose(lib);
				#endif
			}
		}

		// Loads library handle from local dynamic library
		void bind_library(const std::string& filename)
		{
			if(lib)
			{
				err_glob += "plugin: a library is already loaded! overwriting is not alllowed.\n";
				return;
			}

			#if OS_IS_WINDOWS
			lib = (void*)LoadLibrary(filename.c_str());
			#else
			lib = dlopen(filename.c_str(), RTLD_NOW);
			#endif

			if(!lib)
			{
				#if OS_IS_WINDOWS
				err_glob += GetLastError();
				#else
				err_glob += dlerror();
				#endif
				err_glob += '\n';
			}
		}

		// loads function into function pointer map
		void bind_function(const std::string& funcname)
		{
			// function pointer that will be stored
			func_ptr func;

			//guard against unloaded library
			if(!lib)
			{
				err_glob += filename + ": lib has not been bound! cannot continue with binding function: '" + funcname + "()'.\n";
				return;
			}

			//loading function from library
			#if OS_IS_WINDOWS
			func = (func_ptr)GetProcAddress((HINSTANCE)lib, funcname.c_str());
			#else
			func = (func_ptr)dlsym(lib, funcname.c_str());
			#endif

			// guard against unbound function
			if(!func)
			{
				#if OS_IS_WINDOWS
				err_glob += GetLastError();
				#else
				err_glob += dlerror();
				#endif
				err_glob += '\n';
			}

			// putting function into map
			functions[funcname] = func;
		}

		// calls function from plugin's map
		template <typename T, typename ...Args>
		T execute(const std::string& funcname, Args... a)
		{
			T(*func)(Args...) = (T(*)(Args...))functions[funcname];
			// guard against function
			if(!func)
			{
				err_glob += filename + ": function '" + funcname + "()' is not bound!\n";
				return T();
			}
			else
			{
				return func(a...);
			}
		}
		inline bool is_lib_bound() const { return (lib != nullptr); }
		inline bool is_func_bound(const std::string& funcname) const { return functions.count(funcname); }
		inline func_ptr get_func(const std::string& funcname)
		{
			return functions[funcname];
		}
		inline std::string get_error()
		{
			std::string out = err_glob;
			err_glob.clear();
			return out;
		}
	};
}
