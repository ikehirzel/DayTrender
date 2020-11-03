pluginlib
========
A C++11 single-header cross-platform plugin handling library

It has no external depencies and therefore will simply work by just including **plugin.h**.

NOTE: Care must be taken when executing functions. They are stored as (void (*)()) function pointers but when executed they are casted to the type specified by the templates. This can cause segmentation faults if the wrong type is used for a stored function

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
	#include "plugin.h"

	using namespace std;

	int main()
	{
		hirzel::Plugin p;

		p.loadLibrary("./exampleplugin.dll");
		p.loadFunction("double_value");
		p.loadFunction("hello");

		// loadLibrary and loadFunction can be handled in the constructor well:
		// tinyplug::Plugin p("exampleplugin.dll", { "double_value", "hello" });

		int dub = p.execute<int, int>("double_value", 5);
		string text = p.execute<string, string>("hello", "Ike");


		cout << dub << endl; // outputs 10
		cout << text << endl; // output 'Hello, Ike!'

		return 0;
	}
```
License
=======
MIT License © 2020 Ike Hirzel
