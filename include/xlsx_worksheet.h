#pragma once

#include <string>
#include <vector>
#include <map>
#include <string_view>
#include "xlsx_types_forward.h"

namespace spiritsaway::xlsx_reader
{
	class worksheet
	{
	public:

		const std::string m_name;
		const std::uint32_t m_sheet_id;
		const void* m_workbook;
		const std::vector<cell>& m_cells;
		std::uint32_t max_rows;
		std::uint32_t max_columns;
		
	public:

		const std::string& get_name() const;
		std::uint32_t get_max_row() const;
		std::uint32_t get_max_column() const;
		worksheet(const std::vector<cell>& all_cells, std::uint32_t in_sheet_id, const std::string& in_sheet_name, const workbook<worksheet>* in_workbook);
		const workbook<worksheet>* get_workbook() const;
		const std::vector<std::vector<std::uint32_t>>& get_all_row() const;
		const std::vector<std::uint32_t>& get_row(std::uint32_t) const;
		std::string_view get_cell(std::uint32_t row_idx, std::uint32_t column_idx) const;
		std::uint32_t get_cell_shared_string_idx(std::uint32_t row_idx, std::uint32_t column_idx) const;
		friend std::ostream& operator<<(std::ostream& output_steam, const worksheet& in_worksheet);
		virtual ~worksheet();
		std::string after_load_process(); //处理load完xml之后的后处理
		void load_from_cells();
		worksheet& operator=(const worksheet& other) = delete;
		worksheet(const worksheet& other) = delete;
	private:
		std::vector<std::vector<std::uint32_t>> row_info;
	};
}