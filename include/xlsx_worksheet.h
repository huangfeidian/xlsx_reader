#pragma once

#include <string>
#include <vector>
#include <map>
#include <string_view>
#include "xlsx_types_forward.h"

namespace xlsx_reader
{
	class worksheet
	{
	public:

		const std::string_view _name;
		const std::uint32_t _sheet_id;
		const void* _workbook;
		const std::vector<cell>& _cells;
		std::uint32_t max_rows;
		std::uint32_t max_columns;
		
	public:

		std::string_view get_name() const;
		std::uint32_t get_max_row() const;
		std::uint32_t get_max_column() const;
		worksheet(const std::vector<cell>& all_cells, std::uint32_t in_sheet_id, std::string_view in_sheet_name, const workbook<worksheet>* in_workbook);
		const workbook<worksheet>* get_workbook() const;
		const std::vector<std::vector<std::uint32_t>>& get_all_row() const;
		const std::vector<std::uint32_t>& get_row(std::uint32_t) const;
		std::string_view get_cell(std::uint32_t row_idx, std::uint32_t column_idx) const;
		friend std::ostream& operator<<(std::ostream& output_steam, const worksheet& in_worksheet);
		virtual ~worksheet();
		void after_load_process(); //处理load完xml之后的后处理
		void load_from_cells();
		std::uint32_t memory_details() const;
		worksheet& operator=(const worksheet& other) = delete;
		worksheet(const worksheet& other) = delete;
	private:
		std::vector<std::vector<std::uint32_t>> row_info;
	};
}