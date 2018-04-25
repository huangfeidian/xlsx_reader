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
        cell get_cell(std::uint32_t row_idx, std::uint32_t:: col_idx) const;
        const workbook& _workbook;
        const std::string_view _name;
        worksheet(const workbook& in_workbook, string_view in_name);
        const workbook& get_workbook()const;
        string_view get_name() const;
        std::uint32_t get_row() const;
        std::uint32_t get_column() const;

    }
}