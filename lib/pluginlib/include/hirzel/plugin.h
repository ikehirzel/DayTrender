/*
	plugin.h
	Copyright 2020 Ike Hirzel
	MIT License
*/

#pragma once

#include <string>
#include <iostream>
#include <map>
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
	class Plugin
	{
	private:
	// Stored handle of library
	void *lib = nullptr;
	// Stores pointers to the functions
	std::map<std::string, void(*)()> functions;

	public:
		Plugin() = default;

		// Constructor that will load library on creation
		Plugin(const std::string& filename)
		{
			loadLibrary(filename);
		}

		Plugin(const std::string& filename, const std::vector<std::string>& funcnames)
		{
			loadLibrary(filename);

			if(lib)
			{
				for(std::string s : funcnames)
				{
					bindFunction(s);
				}
			}
			else
			{
				#ifdef PLUGINLIB_DEBUG
				std::cout << "Plugin::Plugin() : Binding of functions cannot continue!\n";
				#endif
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
		void loadLibrary(const std::string& filename)
		{
			if(lib)
			{
				#ifdef PLUGINLIB_DEBUG
				std::cout << "Plugin::loadLibrary() : A library is already loaded! aborting..." << std::endl;
				#endif
				return;
			}

			#if OS_IS_WINDOWS
			lib = (void*)LoadLibrary(filename.c_str());
			#else
			lib = dlopen(filename.c_str(), RTLD_NOW);
			#endif

			if(!lib)
			{
				#ifdef PLUGINLIB_DEBUG
				std::cout << "Plugin::loadLibrary() : Failed to load library: '" << filename << "'\n";
				#endif
			}
		}

		// loads function into function pointer map
		void bindFunction(const std::string& funcname)
		{
			// function pointer that will be stored
			void (*func)();

			//guard against unloaded library
			if(!lib)
			{
				#ifdef PLUGINLIB_DEBUG
				std::cout << "Plugin::bindFunction() : Library has not been loaded! Cannot continue with loading function: '" + funcname + "()'\n";
				#endif
				return;
			}

			//loading function from library
			#if OS_IS_WINDOWS
			func = (void(*)())GetProcAddress((HINSTANCE)lib, funcname.c_str());
			#else
			func = (void(*)())dlsym(lib, funcname.c_str());
			#endif

			// guard against unbound function
			if(!func)
			{
				#ifdef PLUGINLIB_DEBUG
				std::cout << "Plugin::bindFunction() : Failed to bind to function: '" << funcname << "()'\n";
				#endif
			}

			// putting function into map
			functions[funcname] = func;
		}

		template <typename ...Args>
		void execute(const std::string& funcname, Args... a)
		{
			void (*func)(Args...) = (void(*)(Args...))functions[funcname];
			// guard against function
			if(!func)
			{
				#ifdef PLUGINLIB_DEBUG
				std::cout << "Plugin::execute() : Failed to find function: '" + funcname + "()'\n";
				#endif
			}
			else
			{
				(*func)(a...);
			}
		}

		// calls function from plugin's map
		template <typename T, typename ...Args>
		T execute(const std::string& funcname, Args... a)
		{
			T out;
			T(*func)(Args...) = (T(*)(Args...))functions[funcname];
			// guard against function
			if(!func)
			{
				#ifdef PLUGINLIB_DEBUG
				std::cout << "Plugin::execute() : Failed to find function: '" + funcname + "()'\n";
				#endif
				return out;
			}
			else
			{
				out = (*func)(a...);
				return out;
			}
		}
	};
}
