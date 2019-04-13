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
	public:
		static string_table& get_instance();
		static const std::uint16_t buffer_size = 4096 * 2;
		static const std::uint16_t long_string_size = 4096 / 2;

		std::string_view add(std::string_view other);
		std::string_view add(const std::string& other);
		~string_table();
	private:
		// store buffer pinter and current used size
		// just for small strings
		std::vector<std::pair<char*, std::uint16_t>> buffers;
		// for long strings
		std::vector<std::string_view> long_strings;
		// store allocated buffer address 
		// for contains lookup
		std::unordered_set<std::size_t> buffer_set;
		// store the long strings
		std::unordered_set<std::string_view> str_set;
		bool contains(std::string_view source_str) const;

	};
}
