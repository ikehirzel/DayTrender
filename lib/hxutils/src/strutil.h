#pragma once

#include <vector>
#include <string>

namespace hirzel
{
	std::vector<std::string> tokenize(const std::string& str, const std::string& delim);
	std::string purge_delims(const std::string& str, const std::string& delims);
	std::string replace_delims(const std::string& str, const std::string& delims, char replacement);
	void replace_hook(std::string& str, const std::string& hook, const std::string& replacement);
	std::string get_folder(const std::string& filepath);
	std::string get_filename(const std::string& filepath);
	std::string get_extension(const std::string& filepath);
	std::string get_basename(const std::string& filepath);
}