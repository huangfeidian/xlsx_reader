#pragma once
#include "xlsx_types_forward.h"
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <string_view>

namespace spiritsaway::xlsx_reader
{
	std::string column_index_to_string(std::uint32_t col_idx);
	std::uint32_t column_index_from_string(const std::string& reference_string);
	std::pair<std::uint32_t, std::uint32_t> row_column_tuple_from_string(const std::string& tuple_string);
	std::string row_column_tuple_to_string(std::pair<std::uint32_t, std::uint32_t> row_column_tuple);
	std::vector<std::string_view> split_string(std::string_view input_string, char sep);
	std::optional<double> cast_numeric(std::string_view s);
	std::optional<int> cast_int(std::string_view s);
	std::string_view strip_blank(std::string_view input);
	template <typename T>
	std::optional<T> cast_string_view(std::string_view _text)
	{
		return nullopt;
	}
	template <>
	std::optional<int> cast_string_view<int>(std::string_view _text);
	template <>
	std::optional<bool> cast_string_view<bool>(std::string_view _text);
	template <>
	std::optional<std::uint32_t> cast_string_view<std::uint32_t>(std::string_view _text);
	template <>
	std::optional<std::int64_t> cast_string_view<std::int64_t>(std::string_view _text);
	template <>
	std::optional<std::uint64_t> cast_string_view<std::uint64_t>(std::string_view _text);
	template <>
	std::optional<float> cast_string_view<float>(std::string_view _text);
	template <>
	std::optional<double> cast_string_view<double>(std::string_view _text);

}

