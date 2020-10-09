#pragma once

#include <string>
#include <vector>

namespace hirzel
{
	struct Log
	{
		unsigned int level = 0;
		std::string classname = "";
		std::string msg = "";
		std::string timestamp = "";

		Log(unsigned int level, const std::string& classname, const std::string& msg);

		std::string content();
		void print();
	};

	class Logger
	{
	private:
		std::string classname;
		static std::vector<Log> logs;

		void log(unsigned int level, const std::string& classname, const std::string& msg);
	public:
		Logger() = default;
		Logger(const std::string& classname);

		void info(const std::string& msg);
		void success(const std::string& msg);
		void warning(const std::string& msg);
		void error(const std::string& msg);
		void fatal(const std::string& msg);

		void dump();

		inline void setClassname(const std::string& classname)
		{
			this->classname = classname;
		}
	};
}
