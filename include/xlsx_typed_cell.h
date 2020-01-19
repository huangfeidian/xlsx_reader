#pragma once
#include <string_view>
#include <cstdint>
#include <optional>
#include <memory>
#include <variant>
#include <string>
#include <vector>

#include <typed_string/arena_typed_string.h>

#include "xlsx_types_forward.h"

namespace spiritsaway::xlsx_reader
{
	using spiritsaway::container::arena_typed_value;
	class typed_cell
	{
	public:
		static const int row_begin = 1;
		static const int column_begin = 1;
		const arena_typed_value* cur_arena_typed_value;
		std::uint32_t _row;
		std::uint32_t _column;
		typed_cell(std::uint32_t in_row, std::uint32_t in_column, const arena_typed_value* in_value)
		: _row(in_row)
		, _column(in_column)
		, cur_arena_typed_value(in_value)
		{

		}
		typed_cell(const typed_cell& other) = default;
		typed_cell& operator=(const typed_cell& other) = default;
		template <typename T> 
		std::optional<T> expect_value() const;
	};
	template <typename T>
	std::optional<T> typed_cell::expect_value() const
	{
		if(!cur_arena_typed_value)
		{
			return std::nullopt;
		}
		else
		{
			return cur_arena_typed_value->expect_value<T>();
		}
	}

	
}