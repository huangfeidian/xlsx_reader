#pragma once

#include "xlsx_types.h"
#include <cstdint>
#include <ostream>
#include <optional>
#include <string_view>
namespace xlsx_reader {
	class cell
	{
	public:
		cell(const cell& rhs) = default;
		template <typename T>
		std::optional<T> expect_value() const
		{
			return nullopt;
		}
		std::uint32_t get_column() const;
		std::uint32_t get_row() const;
		std::pair<std::uint32_t, std::uint32_t> get_row_column() const;
		std::string to_string() const;
		cell(uint32_t row_idx, uint32_t col_idx, std::string_view in_text);
		cell &operator=(const cell &rhs);
		bool operator==(const cell &comparand) const;
		friend std::ostream& operator<<(std::ostream& output_steam, const cell& in_cell);
		std::string_view _text;
		std::uint32_t _row;
		std::uint32_t _column;

	};
	template<>
	std::optional<bool> cell::expect_value<bool>() const;
	template<>
	std::optional<std::int32_t> cell::expect_value<std::int32_t>() const;
	template<>
	std::optional<std::uint32_t> cell::expect_value<std::uint32_t>() const;
	template<>
	std::optional<std::int64_t> cell::expect_value<std::int64_t>() const;
	template<>
	std::optional<std::uint64_t> cell::expect_value<std::uint64_t>() const;
	template<>
	std::optional<float> cell::expect_value<float>() const;
	template<>
	std::optional<double> cell::expect_value<double>() const;
	template<>
	std::optional<std::string_view> cell::expect_value<std::string_view>() const;

}