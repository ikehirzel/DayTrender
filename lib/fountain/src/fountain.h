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
		// returns a formatted string
		std::string formatStr(const std::string& str, const std::vector<var>& vars = {});
		// prints string in specified fromat but does not push it to log list, essentially a type safe printf variant
		void printFmt(const std::string& str, const std::vector<var>& vars = {});
		// specifies the output file and on windows enables VT100 control codes
		void init(const std::string& _logfilename, bool _debug_mode = false);
		// pushes the log to the list but does not print it
		void pushLog(unsigned level, const char* name, int line, const std::string& str, const std::vector<var>& vars);
		// pushes the log and the list and prints it
		void printLog(unsigned level, const char* name, int line, const std::string& str, const std::vector<var>& vars);
		// flushes the list into specified output file and empties the list
		void dump();
		// const ref to the log list for parsing
		const std::vector<std::string>& getLogs();
	}
}

// checks if the user wants to use the macros as they may or may not cause name collisions
#ifndef EXCLUDE_FOUNTAIN_MACROS

// macro for printFmt for convenience as it allows for variadic 
#define printfmt(msg, ...)	hirzel::fountain::printFmt(msg, { __VA_ARGS__ });

// the following logs push and print

#define debugf(msg, ...)	hirzel::fountain::printLog(FOUNTAIN_DEBUG, __FILE__, __LINE__, msg, { __VA_ARGS__} )
#define infof(msg, ...)		hirzel::fountain::printLog(FOUNTAIN_INFO, __FILE__, __LINE__, msg, { __VA_ARGS__} )
#define successf(msg, ...)	hirzel::fountain::printLog(FOUNTAIN_SUCCESS, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define warningf(msg, ...)	hirzel::fountain::printLog(FOUNTAIN_WARNING, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define errorf(msg, ...)	hirzel::fountain::printLog(FOUNTAIN_ERROR, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define fatalf(msg, ...)	hirzel::fountain::printLog(FOUNTAIN_FATAL, __FILE__, __LINE__, msg, { __VA_ARGS__ })

// The following macros push but do not print

#define pdebugf(msg, ...)	hirzel::fountain::pushLog(FOUNTAIN_DEBUG, __FILE__, __LINE__, msg, { __VA_ARGS__} )
#define pinfof(msg, ...) 	hirzel::fountain::pushLog(FOUNTAIN_INFO, __FILE__, __LINE__, msg, { __VA_ARGS__} )
#define psuccessf(msg, ...)	hirzel::fountain::pushLog(FOUNTAIN_SUCCESS, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define pwarningf(msg, ...)	hirzel::fountain::pushLog(FOUNTAIN_WARNING, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define perrorf(msg, ...)	hirzel::fountain::pushLog(FOUNTAIN_ERROR, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define pfatalf(msg, ...)	hirzel::fountain::pushLog(FOUNTAIN_FATAL, __FILE__, __LINE__, msg, { __VA_ARGS__ })

#endif