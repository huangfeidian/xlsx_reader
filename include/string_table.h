#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>

namespace xlsx_reader
{
	class string_table
	{
		static string_table& get_instance();
		static const std::uint16_t buffer_size = 4096 * 2;
		static const std::uint16_t long_string_size = 4096 / 2;
		std::vector<std::pair<char*, std::uint16_t>> buffers;
		std::vector<std::string_view> long_strings;
		std::unordered_set<std::size_t> buffer_set;
		std::unordered_set<std::string_view> str_set;
		bool contains(std::string_view source_str) const;
		std::string_view add(std::string_view other);
		std::string_view add(const std::string& other);
		~string_table();

	};
}
