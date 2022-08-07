#pragma once
#include <string_view>
#include <cstdint>
#include <optional>
#include <memory>
#include <variant>
#include <string>
#include <vector>

#include <typed_string/typed_string_desc.h>

#include <any_container/decode.h>

#include "xlsx_types_forward.h"

namespace spiritsaway::xlsx_reader
{
	using spiritsaway::container::typed_string_desc;
	class typed_cell
	{
	public:
		static const int row_begin = 1;
		static const int column_begin = 1;
		const json cur_arena_typed_value;
		std::uint32_t m_row;
		std::uint32_t m_column;
		typed_cell(std::uint32_t in_row, std::uint32_t in_column, const json& in_value)
		: m_row(in_row)
		, m_column(in_column)
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
			T result;
			if (!spiritsaway::serialize::decode(cur_arena_typed_value, result))
			{
				return std::nullopt;
			}
			else
			{
				return std::move(result);
			}
		}
	}

	
}