#pragma once

#include <vector>
#include <string>

namespace hirzel
{
	bool is_invisible(char c);
	// tokenize a string with single char delimiters
	std::vector<std::string> tokenize(const std::string& str, const std::string& delims, bool delim_invisible = false);
	// tok
	std::vector<std::string> tokenize(const std::string& str, const std::string& delims,
		const std::vector<std::string>& preset_tokens, bool delim_invisible = false);

	std::string purge_delims(const std::string& str, const std::string& delims, bool delim_invisible = false);
	std::string replace_delims(const std::string& str, const std::string& delims, char replacement);
	void find_and_purge(std::string& str, const std::string& token);
	void find_and_replace(std::string& str, const std::string& token, const std::string& replacement);

	std::string get_folder(const std::string& filepath);
	std::string get_filename(const std::string& filepath);
	std::string get_extension(const std::string& filepath);
	std::string get_basename(const std::string& filepath);
}