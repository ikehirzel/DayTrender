#include "fountain.h"

#include <hirzel/var.h>

#include <ctime>
#include <fstream>
#include <cstdint>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#define FOUNTAIN_DEBUG_STR		"[DEBUG]  "
#define FOUNTAIN_INFO_STR		"[INFO]   "
#define FOUNTAIN_SUCCESS_STR	"[SUCCESS]"
#define FOUNTAIN_WARNING_STR	"[WARNING]"
#define FOUNTAIN_ERROR_STR		"[ERROR]  "
#define FOUNTAIN_FATAL_STR		"[FATAL]  "

#define FOUNTAIN_LEVELS                                                  \
	{                                                                    \
		FOUNTAIN_DEBUG_STR, FOUNTAIN_INFO_STR, FOUNTAIN_SUCCESS_STR,      \
			FOUNTAIN_WARNING_STR, FOUNTAIN_ERROR_STR, FOUNTAIN_FATAL_STR \
	}

#define color(x) "\033["#x"m"

#define FOUNTAIN_RESET		color(0)
#define FOUNTAIN_WHITE		color(37)
#define FOUNTAIN_GREEN		color(32)
#define FOUNTAIN_YELLOW		color(33)
#define FOUNTAIN_RED		color(1;31)
#define FOUNTAIN_BRIGHT_RED	color(31)
#define FOUNTAIN_BLUE		color(34)

#define FOUNTAIN_COLORS											\
	{															\
		 FOUNTAIN_BLUE, FOUNTAIN_RESET, FOUNTAIN_GREEN,			\
			FOUNTAIN_YELLOW, FOUNTAIN_RED, FOUNTAIN_BRIGHT_RED	\
	}

#define FOUNTAIN_NAME_BUF_LEN 24

#ifndef FOUNTAIN_TAB_SIZE
#define FOUNTAIN_TAB_SIZE 4
#endif

#define FOUNTAIN_INT		'd'
#define FOUNTAIN_UINT		'u'
#define FOUNTAIN_OCTAL		'o'
#define FOUNTAIN_HEXUPPER	'X'
#define FOUNTAIN_HEXLOWER	'x'
#define FOUNTAIN_FLOAT 		'f'
#define FOUNTAIN_BOOL		'b'
#define FOUNTAIN_STRING		's'
#define FOUNTAIN_CHAR		'c'
#define FOUNTAIN_PERCENT	'%'
// defined for future use
#define FOUNTAIN_POINTER	'p'

namespace hirzel
{
	namespace fountain
	{
		namespace details
		{
			// does not do unary numbers only binary through hexadecimal
			std::string itos(uintmax_t num, unsigned base, bool upper = true)
			{
				if (base > 16 || base < 2)
				{
					return "";
				}
				std::string rev, out;
				unsigned size, index;
				size = sizeof(uintmax_t) * 8;
				index = 0;
				rev.resize(size);
				const char* charset;
				if(upper)
				{
					charset = "0123456789ABCDEF";
				}
				else
				{
					charset = "0123456789abcdef";
				}

				while (num > 0)
				{
					
					rev[index] = charset[num % base];
					num /= base;
					index++;
				}
				out.resize(index);
				unsigned revi = 0;

				for (int i = index - 1; i >= 0; i--)
				{
					out[i] = rev[revi];
					revi++;
				}

				return out;
			}
		}

		std::string logfilename;
		std::vector<std::string> logs;
		bool debug_mode = false;
		const char *colors[] = FOUNTAIN_COLORS;
		const char *levels[] = FOUNTAIN_LEVELS;

		std::string formatStr(const std::string &str, const std::vector<var> &vars)
		{
			std::string out;
			std::string tmp;

			size_t oi = 0, li = 0;
			out.resize(str.size(), 0);

			for (size_t i = 0; i < str.size(); i++)
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
				if (li > vars.size() - 1)
				{
					throw std::out_of_range("Not enough vars supplied to fulfill format!");
					return out;
				}

				/*
					NOTE:

					the reason why the the std::to_string() conversion of the explicit type
					as opposed to the var::as_string() is so that it can throw an exception
					if the incorrect type is used and allows the user to do something like
					interpret an integer as bool. Alternatively, %s could be used and 
					var::as_string will be called anyway if the user would prefer a less
					explicit way of handling the conversion

				*/

				switch (str[i])
				{
				case FOUNTAIN_INT:
					tmp = std::to_string(vars[li].as_int());
					break;

				case FOUNTAIN_UINT:
					tmp = std::to_string(vars[li].as_uint());
					break;

				case FOUNTAIN_OCTAL:
					tmp = details::itos(vars[li].as_uint(), 8);
					break;

				case FOUNTAIN_HEXUPPER:
					tmp = details::itos(vars[li].as_uint(), 16);
					break;

				case FOUNTAIN_HEXLOWER:
					tmp = details::itos(vars[li].as_uint(), 16, false);
					break;

				case FOUNTAIN_FLOAT:
					tmp = std::to_string(vars[li].as_double());
					break;

				case FOUNTAIN_BOOL:
					tmp = vars[li].as_bool() ? "true" : "false";
					break;

				case FOUNTAIN_STRING:
					tmp = vars[li].as_string();
					break;

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

		void printFmt(const std::string &str, const std::vector<var> &vars)
		{
			std::string out = formatStr(str, vars);
			for (size_t i = 0; i < out.size(); i++)
			{
				putchar(out[i]);
			}
		}

		void init(const std::string &_logfilename, bool _debug_mode)
		{

			hirzel::fountain::logfilename = _logfilename;
			hirzel::fountain::debug_mode = _debug_mode;
#if defined(_WIN32) || defined(_WIN64)
			DWORD outMode = 0;
			HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleMode(outHandle, &outMode);
			SetConsoleMode(outHandle, outMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
		}

		void pushLog(unsigned level, const char *name, int line, const std::string &str, const std::vector<var> &list)
		{
			if(level == FOUNTAIN_DEBUG && !debug_mode)
			{
				return;
			}
			time_t currtime;
			std::time(&currtime);
			char timebuf[17];
			std::strftime(timebuf, 16, "%T", localtime(&currtime));
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

			for (char &c : name_buf)
			{
				if (c == 0 || c == 32)
				{
					c = '.';
				}
			}
			
			msg = formatStr(str, list);
			log = formatStr("%s| %s | [%s] %s : %s\n%s", { colors[level], name_buf, std::string(timebuf), levels[level], msg, FOUNTAIN_RESET});
			logs.push_back(log);
		}

		void printLog(unsigned level, const char *name, int line, const std::string &str, const std::vector<var> &list)
		{
			if(level == FOUNTAIN_DEBUG && !debug_mode)
			{
				return;
			}
			pushLog(level, name, line, str, list);
			std::string log = logs.back();
			for (unsigned i = 0; i < log.size(); i++)
			{
				putchar(log[i]);
			}
		}

		void dump()
		{
			if (logfilename.empty())
			{
				puts("The logger has not been initialized!");
				return;
			}

			std::ofstream file;
			file.open(logfilename);

			for (std::string l : logs)
			{
				std::string::iterator start = l.begin(), end = l.end();

				while(*start != '|')
				{
					start++;
				}

				while(*end != '\n')
				{
					end--;
				}

				file << std::string(start, end + 1);
			}

			logs.clear();
			file.close();
		}

		const std::vector<std::string>& getLogs()
		{
			return logs;
		}

	} // namespace fountain
} // namespace hirzel
