#pragma once
#include "xlsx_types_forward.h"
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <string_view>

namespace xlsx_reader
{
	std::string column_index_to_string(std::uint32_t col_idx);
	std::uint32_t column_index_from_string(const std::string& reference_string);
	std::pair<std::uint32_t, std::uint32_t> row_column_tuple_from_string(const std::string& tuple_string);
	std::string row_column_tuple_to_string(std::pair<std::uint32_t, std::uint32_t> row_column_tuple);
	std::vector<std::string_view> split_string(std::string_view input_string, char sep);
	std::optional<double> cast_numeric(std::string_view s);
	std::optional<int> cast_int(std::string_view s);
}

