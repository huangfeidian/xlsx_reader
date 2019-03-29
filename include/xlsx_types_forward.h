#pragma once
#include <string>
#include <cstdint>
#include <tuple>

namespace xlsx_reader
{
	class worksheet;
	template <typename worksheet_t> 
	class workbook;
	class archive;
	class typed_worksheet;
	using relation_desc = std::tuple<std::string, std::string, std::string>;
	using sheet_desc = std::tuple<std::string, std::uint32_t, std::string>;
	using cell = std::tuple<std::uint32_t, std::uint32_t, std::string_view>;
}