#include "strutil.h"
#include <iostream>
namespace hirzel
{
	bool is_invisible(char c)
	{
		return (c < 33) || (c == 127);
	}

	std::vector<std::string> tokenize(const std::string& str, const std::string& delims, bool delim_invisible)
	{
		std::string token;
		std::vector<std::string> tokens;

		for (char c : str)
		{
			bool is_delim = false;
			if (delim_invisible)
			{
				is_delim |= is_invisible(c);
			}
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
				if (token.size() > 0)
				{
					tokens.push_back(token);
				}
				token.clear();
				continue;
			}
			token += c;
		}

		if (token.size() > 0)
		{
			tokens.push_back(token);
		}

		return tokens;
	}

	std::vector<std::string> tokenize(const std::string& str, const std::string& delims,
		const std::vector<std::string>& preset_tokens, bool delim_invisible)
	{
		std::string token;
		std::vector<std::string> tokens;

		for (size_t i = 0; i < str.size(); i++)
		{
			int token_index = -1;
			// check if the next chunk is a save delim
			for (unsigned j = 0; j < preset_tokens.size(); j++)
			{
				const std::string& del = preset_tokens[j];
				// test
				if (str[i] == del[0])
				{
					unsigned end = i + del.size();
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
						if (str[c] != del[di])
						{
							equal = false;
							break;
						}
						di++;
					}

					if (equal)
					{
						// checking if the index has been set yet
						if (token_index < 0 || (token_index >= 0 && del.size() > preset_tokens[token_index].size()))
						{
							token_index = j;
						}
					}


				}
			}
			// delim is set
			if (token_index >= 0)
			{
				if (token.size() > 0)
				{
					tokens.push_back(token);
				}
				token.clear();
				tokens.push_back(preset_tokens[token_index]);
				i += preset_tokens[token_index].size() - 1;
				continue;
			}

			// checking if char is a delimiter to be ignored
			bool is_delim = false;
			if (delim_invisible)
			{
				is_delim |= is_invisible(str[i]);
			}

			for (char d : delims)
			{
				if (d == str[i])
				{
					is_delim = true;
					break;
				}
			}
			// if is delim, take chunk and as token
			if (is_delim)
			{
				if (token.size() > 0)
				{
					tokens.push_back(token);
				}
				token.clear();
				continue;
			}

			

			token += str[i];
		}

		if (token.size() > 0)
		{
			tokens.push_back(token);
		}

		return tokens;
	}

	std::string purge_delims(const std::string& str, const std::string& delims, bool delim_invisible)
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

	std::string replace_delims(const std::string& str, const std::string& delims, char replacement)
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

	void find_and_purge(std::string& str, const std::string& token)
	{
		size_t pos;
		while((pos = str.find(token)) != std::string::npos)
		{
			str.erase(pos, token.size());
		}
	}

	void find_and_replace(std::string& str, const std::string& token, const std::string& replacement)
	{
		size_t pos;
		while((pos = str.find(token)) != std::string::npos)
		{
			str.replace(pos, token.size(), replacement);

		}
	}

	std::string get_folder(const std::string& filepath)
	{
		unsigned int slashIndex = -1;
		for (unsigned int i = 0; i < filepath.size(); i++)
		{
			if(filepath[i] == '/')
			{
				slashIndex = i;
			}
		}
		if(slashIndex < filepath.size())
		{
			return filepath.substr(0, slashIndex);
		}

		return ".";
	}

	std::string get_filename(const std::string& filepath)
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

	std::string get_extension(const std::string& filepath)
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

	std::string get_basename(const std::string& filepath)
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
}