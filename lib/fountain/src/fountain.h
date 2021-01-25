#pragma once

#include <hirzel/var.h>
#include <vector>

#define FOUNTAIN_DEBUG		0
#define FOUNTAIN_INFO		1
#define FOUNTAIN_SUCCESS	2
#define FOUNTAIN_WARNING	3
#define FOUNTAIN_ERROR		4
#define FOUNTAIN_FATAL		5

namespace hirzel
{
	namespace fountain
	{
		// specifies the output file and on windows enables VT100 control codes
		void init(const std::string& log_filename, bool debug_mode, bool print_logs, bool push_logs, int max_log_size);
		// returns a formatted string
		std::string format_str(const std::string& str, const std::vector<var>& vars = {});
		// prints string in specified fromat but does not push it to log list, essentially a type safe printf variant
		void print_fmt(const std::string& str, const std::vector<var>& vars = {});
		// prints / pushes log based on settings
		void log(unsigned level, const char* name, int line, const std::string& str, const std::vector<var>& vars);
		// flushes the list into specified output file and empties the list
		bool dump();
		// const ref to the log list for parsing
		const std::vector<std::string>& logs();
		const char* error();
	}
}

// checks if the user wants to use the macros as they may or may not cause name collisions
#ifndef EXCLUDE_FOUNTAIN_MACROS

// macro for printFmt for convenience as it allows for variadic 
#define printfmt(msg, ...)	hirzel::fountain::print_fmt(msg, { __VA_ARGS__ });

#define debugf(msg, ...)	hirzel::fountain::log(FOUNTAIN_DEBUG, __FILE__, __LINE__, msg, { __VA_ARGS__} )
#define infof(msg, ...)		hirzel::fountain::log(FOUNTAIN_INFO, __FILE__, __LINE__, msg, { __VA_ARGS__} )
#define successf(msg, ...)	hirzel::fountain::log(FOUNTAIN_SUCCESS, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define warningf(msg, ...)	hirzel::fountain::log(FOUNTAIN_WARNING, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define errorf(msg, ...)	hirzel::fountain::log(FOUNTAIN_ERROR, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define fatalf(msg, ...)	hirzel::fountain::log(FOUNTAIN_FATAL, __FILE__, __LINE__, msg, { __VA_ARGS__ })

#endif