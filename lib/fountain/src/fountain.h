#pragma once

#include <hirzel/var.h>
#include <vector>

#define FOUNTAIN_INFO		0
#define FOUNTAIN_SUCCESS	1
#define FOUNTAIN_WARNING	2
#define FOUNTAIN_ERROR		3
#define FOUNTAIN_FATAL		4

namespace hirzel
{
	std::string formatstr(const std::string& str, const std::vector<var>& vars = {});
	void printf(const std::string& str, const std::vector<var>& vars = {});
	void log_init(const std::string& _logfilename);
	void log_push(int level, const char* name, int line, const std::string& str, const std::vector<var>& vars);
	void log_dump();
}

#define infof(msg, ...) hirzel::log_push(FOUNTAIN_INFO, __FILE__, __LINE__, msg, { __VA_ARGS__} )
#define successf(msg, ...) hirzel::log_push(FOUNTAIN_SUCCESS, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define warningf(msg, ...) hirzel::log_push(FOUNTAIN_WARNING, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define errorf(msg, ...) hirzel::log_push(FOUNTAIN_ERROR, __FILE__, __LINE__, msg, { __VA_ARGS__ })
#define fatalf(msg, ...) hirzel::log_push(FOUNTAIN_FATAL, __FILE__, __LINE__, msg, { __VA_ARGS__ })
