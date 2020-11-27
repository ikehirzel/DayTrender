#include "strutil.h"
#include <iostream>
namespace hirzel
{
	namespace str
	{
		bool is_invisible(unsigned char c)
		{
			return (c < 33) || (c == 127);
		}

		bool is_alpha(unsigned char c)
		{
			return (c > 64  && c < 91) || (c > 96 && c < 123);
		}

		bool is_digit(unsigned char c)
		{
			return (c > 47 && c < 58);
		}

		bool is_special(unsigned char c)
		{
			return (c > 32 && c < 48) || (c > 57 && c < 65) || (c > 90 && c < 97) || (c > 122 && c < 127);
		}

		std::vector<std::string> tokenize(const std::string &str, const std::string &delims,
			bool ignore_invisible, bool save_delims)
		{
			std::string token;
			std::vector<std::string> tokens;

			for (char c : str)
			{
				bool is_delim = false;
				for (char d : delims)
				{
					if (d == c)
					{
						is_delim = true;
						break;
					}
				}

				if (is_delim)
				{
					if (!token.empty())
					{
						tokens.push_back(token);
						token.clear();
					}
					if (save_delims)
					{
						tokens.push_back(std::string(1, c));
					}
					continue;
				}

				if (ignore_invisible && is_invisible(c))
					continue;

				token += c;
			}

			if (!token.empty())
			{
				tokens.push_back(token);
			}

			return tokens;
		}

		std::vector<std::string> tokenize(const std::string &str, const std::vector<std::string> &delims,
			bool ignore_invisible, bool save_delims)
		{
			std::string token;
			std::vector<std::string> tokens;

			for (size_t i = 0; i < str.size(); i++)
			{
				int delim_index = -1;
				// check if the next chunk is a save delim
				for (unsigned j = 0; j < delims.size(); j++)
				{
					// test the first character of the delimiters
					if (str[i] == delims[j][0])
					{
						unsigned end = i + delims[j].size();
						// the rest of the string is not long enough to be that token
						if (end > str.size())
						{
							continue;
						}
						bool equal = true;
						unsigned di = 0;
						// comparing the two strings
						for (unsigned c = i; c < end; c++)
						{
							if (str[c] != delims[j][di])
							{
								if (ignore_invisible && is_invisible(str[c]))
								{
									i++;
									end++;
									if (end <= str.size())
										continue;
								}
								equal = false;
								break;
							}
							di++;
						}

						if (equal)
						{
							// checking if the index has been set yet
							if (delim_index < 0 || (delim_index >= 0 && delims[j].size() > delims[delim_index].size()))
							{
								delim_index = j;
							}
						}
					}
				}

				// delim is set
				if (delim_index >= 0)
				{
					if (token.size() > 0)
					{
						tokens.push_back(token);
						token.clear();
					}
					if (save_delims)
					{
						tokens.push_back(delims[delim_index]);
					}
					i += delims[delim_index].size() - 1;
					continue;
				}

				if (ignore_invisible && is_invisible(str[i]))
					continue;

				token += str[i];
			}

			if (token.size() > 0)
			{
				tokens.push_back(token);
			}

			return tokens;
		}

		std::string purge_delims(const std::string &str, const std::string &delims, bool delim_invisible)
		{
			std::string out = str;

			for (unsigned int i = out.size() - 1; i < out.size(); i--)
			{
				bool is_delim = false;
				if (delim_invisible)
				{
					is_delim |= is_invisible(out[i]);
				}
				for (char c : delims)
				{
					if (out[i] == c)
					{
						is_delim = true;
					}
				}
				if (is_delim)
				{
					out.erase(i, 1);
				}
			}

			return out;
		}

		std::string replace_delims(const std::string &str, const std::string &delims, char replacement)
		{
			std::string out = str;

			for (unsigned int i = 0; i < out.size(); i++)
			{
				bool is_delim = false;
				for (char c : delims)
				{
					if (out[i] == c)
					{
						is_delim = true;
					}
				}
				if (is_delim)
				{
					out[i] = replacement;
				}
			}

			return out;
		}

		void find_and_purge(std::string &str, const std::string &token)
		{
			size_t pos;
			while ((pos = str.find(token)) != std::string::npos)
			{
				str.erase(pos, token.size());
			}
		}

		void find_and_replace(std::string &str, const std::string &token, const std::string &replacement)
		{
			size_t pos;
			while ((pos = str.find(token)) != std::string::npos)
			{
				str.replace(pos, token.size(), replacement);
			}
		}

		std::string get_folder(const std::string &filepath)
		{
			unsigned int slashIndex = -1;
			for (unsigned int i = 0; i < filepath.size(); i++)
			{
				if (filepath[i] == '/')
				{
					slashIndex = i;
				}
			}
			if (slashIndex < filepath.size())
			{
				return filepath.substr(0, slashIndex);
			}

			return ".";
		}

		std::string get_filename(const std::string &filepath)
		{
			int slashIndex = -1;
			for (unsigned int i = 0; i < filepath.size(); i++)
			{
				if (filepath[i] == '/')
				{
					slashIndex = i;
				}
			}

			return filepath.substr(slashIndex + 1);
		}

		std::string get_extension(const std::string &filepath)
		{
			int slashIndex = -1, dotIndex = -1;
			for (unsigned int i = 0; i < filepath.size(); i++)
			{
				if (filepath[i] == '/')
				{
					slashIndex = i;
					dotIndex = slashIndex;
				}
				if (filepath[i] == '.' && dotIndex == slashIndex)
				{
					dotIndex = i;
				}
			}

			return filepath.substr(dotIndex + 1);
		}

		std::string get_basename(const std::string &filepath)
		{
			int slashIndex = -1, dotIndex = -1;
			for (unsigned int i = 0; i < filepath.size(); i++)
			{
				if (filepath[i] == '/')
				{
					slashIndex = i;
					dotIndex = slashIndex;
				}
				if (filepath[i] == '.' && dotIndex == slashIndex)
				{
					dotIndex = i;
				}
			}

			return filepath.substr(slashIndex + 1, dotIndex - slashIndex - 1);
		}
	} // namespace strutil
} // namespace hirzel