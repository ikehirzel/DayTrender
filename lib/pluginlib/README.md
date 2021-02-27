pluginlib
========
A C++11 single-header cross-platform plugin handling library

It has no external depencies and therefore will simply work by just including **plugin.h**.

NOTE: Care must be taken when executing functions. Types of functions are not stored and 
are up to the user to handle

Examples
========
#### Example plugin
```c++
	// contents of exampleplugin.cpp
	#include <string>

	// extern "C" is required to avoid name mangling
	extern "C"
	{
		int double_value(int val)
		{
			return val * 2;
		}

		std::string hello(const std::string& name)
		{
			return "Hello, " + name + "!";
		}
	}
```
#### Driver code
```c++
	// contents of main.cpp
	#include <iostream>
	#include <string>
	#include <hirzel/plugin.h>

	int main()
	{
		hirzel::Plugin p;

		p.bind_library("./exampleplugin.dll");
		p.bind_function("double_value");
		p.bind_function("hello");

		// loadLibrary and loadFunction can be handled in the constructor well:
		// hirzel::Plugin p("./exampleplugin.dll", { "double_value", "hello" });

		int dub = p.execute<int, int>("double_value", 5);
		string text = p.execute<string, string>("hello", "Ike");


		std::cout << dub << std::endl; // outputs 10
		std::cout << text << std::endl; // output 'Hello, Ike!'

		return 0;
	}
```
License
=======
MIT License © 2020 Ike Hirzel
