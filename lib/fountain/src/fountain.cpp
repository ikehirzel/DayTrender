#include "fountain.h"

#include <hirzel/var.h>

#include <ctime>
#include <fstream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#define FOUNTAIN_INFO_STR		"[INFO]   "
#define FOUNTAIN_SUCCESS_STR	"[SUCCESS]"
#define FOUNTAIN_WARNING_STR	"[WARNING]"
#define FOUNTAIN_ERROR_STR		"[ERROR]  "
#define FOUNTAIN_FATAL_STR		"[FATAL]  "

#define FOUNTAIN_LEVELS { FOUNTAIN_INFO_STR, FOUNTAIN_SUCCESS_STR,\
	FOUNTAIN_WARNING_STR, FOUNTAIN_ERROR_STR, FOUNTAIN_FATAL_STR }

#define color(x) 			"\033["#x"m"
#define FOUNTAIN_RESET		color(0)
#define FOUNTAIN_WHITE		color(37)
#define FOUNTAIN_GREEN		color(32)
#define FOUNTAIN_YELLOW		color(33)
#define FOUNTAIN_RED		color(31)
#define FOUNTAIN_BRIGHT_RED	color(1;31)

#define FOUNTAIN_COLORS { FOUNTAIN_RESET, FOUNTAIN_GREEN,\
	FOUNTAIN_YELLOW, FOUNTAIN_RED, FOUNTAIN_BRIGHT_RED }

#define FOUNTAIN_NAME_BUF_LEN	24

#ifndef FOUNTAIN_TAB_SIZE
#define FOUNTAIN_TAB_SIZE		4
#endif

#define FOUNTAIN_STRING			's'
#define FOUNTAIN_CHAR			'c'
#define FOUNTAIN_INT			'd'
#define FOUNTAIN_UINT			'u'
#define FOUNTAIN_OCTAL			'o'
#define FOUNTAIN_HEXLOWER		'x'
#define FOUNTAIN_HEXUPPER		'X'
#define FOUNTAIN_FLOAT			'f'
#define FOUNTAIN_PERCENT		'%'

namespace hirzel
{
	static std::string logfilename;
	static std::vector<std::string> logs;
	
	const char *colors[] = FOUNTAIN_COLORS;
	const char *levels[] = FOUNTAIN_LEVELS;

	std::string formatstr(const std::string& str, const std::vector<var>& vars)
	{
		std::string out;
		std::string tmp;

		size_t oi = 0, li = 0;
		out.resize(str.size(), 0);

		for(size_t i = 0; i < str.size(); i++)
		{
			while (str[i] != '%' && i < str.size())
			{
				out[oi] = str[i];
				oi++;
				i++;
			}

			i++;
			if (i > str.size())
			{
				break;
			}

			tmp.clear();
			if(li > vars.size() - 1)
			{
				throw std::out_of_range("Not enough vars supplied to fulfill format!");
				return out;
			}
			switch(str[i])
			{
				case FOUNTAIN_INT:
					tmp = std::to_string(vars[li].as_int());
					break;

				case FOUNTAIN_UINT:
					tmp = std::to_string(vars[li].as_uint());
					break;

				case FOUNTAIN_STRING:
					tmp = vars[li].as_string();
					break;

				case FOUNTAIN_FLOAT:
					tmp = std::to_string(vars[li].as_double());
					break;

				// the following cases skip the string copy section for efficiency
				case FOUNTAIN_CHAR:
					out[oi] = vars[li].as_char();
					oi++;
					li++;
					continue;
				
				case FOUNTAIN_PERCENT:
					out[oi] = '%';
					oi++;
					continue;
			}

			//resizing to avoid out of bounds
			out.resize(out.size() + tmp.size());

			// contents of tmp will be copied into out
			for (size_t j = 0; j < tmp.size(); j++)
			{
				out[oi] = tmp[j];
				oi++;
			}

			li++;
		}

		out.shrink_to_fit();
		return out;
	}

	void printf(const std::string& str, const std::vector<var>& vars)
	{
		std::string out = formatstr(str, vars);
		for (size_t i = 0; i < out.size(); i++)
		{
			putchar(out[i]);
		}
	}

	void log_init(const std::string& _logfilename)
	{
		hirzel::logfilename = _logfilename;
#if defined(_WIN32) || defined(_WIN64)
		DWORD outMode = 0;
		HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleMode(outHandle, &outMode);
		SetConsoleMode(outHandle, outMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
	}

	void log_push(int level, const char* name, int line, const std::string& str, const std::vector<var>& list)
	{

		unsigned long timestamp = time(NULL);
		std::string name_str, log, msg, line_buf;
		std::string name_buf(FOUNTAIN_NAME_BUF_LEN, ' ');
		
		line_buf = std::to_string(line);
		name_str = std::string(name) + ':' + line_buf;
		name_str = name_buf + name_str;
		int spaces = FOUNTAIN_NAME_BUF_LEN - name_str.size();

		for (int i = 0; i < FOUNTAIN_NAME_BUF_LEN; i++)
		{
			name_buf[i] = name_str[i - spaces];
		}

		for(char &c : name_buf)
		{
			if(c == 0 || c == 32)
			{
				c = '.';
			}
		}

		msg = formatstr(str, list);
		log = formatstr("| %s | [%d] %s : %s\n", { name_buf, timestamp, levels[level], msg});

		const char *pos = nullptr;

		for (pos = colors[level]; *pos != 0; pos++)
		{
			putchar(*pos);
		}

		for (size_t i = 0; i < log.size(); i++)
		{
			putchar(log[i]);
		}

		for (pos = FOUNTAIN_RESET; *pos != 0; pos++)
		{
			putchar(*pos);
		}

		logs.push_back(log);
	}
	
	void log_dump()
	{
		if(logfilename.empty())
		{
			puts("The logger has not been initialized!");
			return;
		}
		
		std::ofstream file;
		file.open(logfilename);

		for (std::string l : logs)
		{
			file << l << std::endl;
		}

		file.close();
	}
}
