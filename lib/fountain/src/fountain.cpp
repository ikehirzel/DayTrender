#include "fountain.h"

#include <hirzel/var.h>

#include <ctime>
#include <fstream>
#include <cstdint>
#include <mutex>
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
#define FOUNTAIN_PTRLOWER	'p'
#define FOUNTAIN_PTRUPPER	'P'

namespace hirzel
{
	namespace fountain
	{
		namespace details
		{
			// minimum of base-2
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

		std::mutex _mtx;
		const char* _error = nullptr;
		std::string _log_filename;
		std::vector<std::string> _logs;
		int _max_log_size = 0;
		bool _print_logs = false;
		bool _push_logs = false;
		bool _debug_mode = false;
		const char *_colors[] = FOUNTAIN_COLORS;
		const char *_levels[] = FOUNTAIN_LEVELS;

		std::string format_str(const std::string &str, const std::vector<var> &vars)
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
					tmp = "0x" + details::itos(vars[li].as_uint(), 16);
					break;

				case FOUNTAIN_HEXLOWER:
					tmp = "0x" + details::itos(vars[li].as_uint(), 16, false);
					break;
				
				case FOUNTAIN_PTRUPPER:
					tmp = "0x" + details::itos((uintmax_t)vars[li].as_bytes(), 16);
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

		void print_fmt(const std::string &str, const std::vector<var> &vars)
		{
			std::string out = format_str(str, vars);
			for (size_t i = 0; i < out.size(); i++)
			{
				putchar(out[i]);
			}
		}

		void init(const std::string& log_filename, bool debug_mode, bool print_logs, bool push_logs, int max_log_size)
		{
			_log_filename = log_filename;
			_push_logs = push_logs;
			_print_logs = print_logs;
			_max_log_size = max_log_size;
			_debug_mode = debug_mode;
			if (!_log_filename.empty())
			{
				std::ofstream file;
				file.open(_log_filename);
				if (file.is_open())
				{
					file << "---- START OF LOG FILE ----\n";
				}
			}
#if defined(_WIN32) || defined(_WIN64)
			DWORD outMode = 0;
			HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleMode(outHandle, &outMode);
			SetConsoleMode(outHandle, outMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
		}

		void log(unsigned level, const char *name, int line, const std::string &str, const std::vector<var> &list)
		{
			if(level == FOUNTAIN_DEBUG && !_debug_mode)
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
			
			msg = format_str(str, list);
			log = format_str("%s| %s | [%s] %s : %s" FOUNTAIN_RESET "\n", { _colors[level], name_buf, std::string(timebuf), _levels[level], msg });

			if (_push_logs)
			{
				_mtx.lock();
				_logs.push_back(log);
				if (_logs.size() >= _max_log_size)
				{

					dump();
				}
				_mtx.unlock();
			}

			if (_print_logs)
			{
				_mtx.lock();
				for (int i = 0; i < log.size(); i++)
				{
					std::putchar(log[i]);
				}
				_mtx.unlock();
			}
		}

		bool dump()
		{
			if (_log_filename.empty())
			{
				_error = "The output filename has not been set";
				return false;
			}

			if (_logs.empty())
			{
				_error = "There were no logs to dump";
				return false;
			}

			std::ofstream file;
			file.open(_log_filename, std::ios_base::app);

			if (!file.is_open())
			{
				_error = "The file failed to open";
				return false;
			}

			for (std::string l : _logs)
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

			_logs.clear();
			file.close();
			return true;
		}

		const std::vector<std::string>& logs() { return _logs; }
		const char* error() { return _error; }
	}
}
