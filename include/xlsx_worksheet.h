#pragma once

#include <string>
#include <vector>
#include <xlsx_cell.h>
#include <map>
#include <string_view>
namespace xlsx_reader
{
	class worksheet
	{
	public:

		const std::string_view _name;
		const std::uint32_t _sheetId;
		const workbook& get_workbook() const;
		std::string_view get_name() const;
		std::uint32_t get_max_row() const;
		std::uint32_t get_max_column() const;
		worksheet(const workbook* in_workbook, std::uint32_t in_sheet_id, std::string_view in_sheet_name);
		const std::map<std::uint32_t, const cell*>& get_row(std::uint32_t) const;
		const cell* get_cell(std::uint32_t row_idx, std::uint32_t column_idx) const;
		friend std::ostream& operator<<(std::ostream& output_steam, const worksheet& in_worksheet);
	private:
		std::vector<cell> _cells;
		std::uint32_t max_rows;
		std::uint32_t max_columns;
		const workbook* _workbook;
		std::map<std::uint32_t, std::map<std::uint32_t, const cell*>> row_info;
		void load_worksheet_from_string(std::string_view _input_string);
	};
}