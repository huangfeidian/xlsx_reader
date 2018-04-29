#pragma once

#include <string>
#include <vector>
#include <xlsx_cell.h>

namespace xlsx_reader
{
    class worksheet
    {
    public:
        std::vector<std::vector<cell>> _cells;
        std::uint32_t _rows;
        std::uint32_t _columns;
        const cell& get_cell(std::uint32_t row_idx, std::uint32_t col_idx) const;
        const std::string_view _name;
        const std::uint32_t _sheetId;
        const workbook& get_workbook()const;
        string_view get_name() const;
        std::uint32_t get_row() const;
        std::uint32_t get_column() const;
        worksheet(const workbook* in_workbook, std::uint32_t in_sheet_id, std::string_view in_sheet_name);
    private:
        const workbook* _workbook;
        void load_worksheet_from_string(std::string_view _input_string);
    }
}