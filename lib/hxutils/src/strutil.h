#pragma once

#include <vector>
#include <string>

// ignore_invisible should make it so that the invisible chars were never in the file to begin with
// tokenizing will deal with this

namespace hirzel
{
	namespace str
	{
		bool is_invisible(unsigned char c);
		bool is_alpha(unsigned char c);
		bool is_digit(unsigned char c);
		bool is_special(unsigned char c);

		// tokenize a string with single char delimiters
		std::vector<std::string> tokenize(const std::string& str, const std::string& delims,
			bool ignore_invisible = false, bool save_delims = false);

		// tokenize a string with multi-char delimiters
		std::vector<std::string> tokenize(const std::string& str, const std::vector<std::string>& delims,
			bool ignore_invisible = false, bool save_delims = false);

		std::string purge_delims(const std::string& str, const std::string& delims, bool delim_invisible = false);
		std::string replace_delims(const std::string& str, const std::string& delims, char replacement);

		void find_and_purge(std::string& str, const std::string& token);
		void find_and_replace(std::string& str, const std::string& token, const std::string& replacement);

		std::string get_folder(const std::string& filepath);
		std::string get_filename(const std::string& filepath);
		std::string get_extension(const std::string& filepath);
		std::string get_basename(const std::string& filepath);
	}
}