#include <xlsx_worksheet.h>
#include <xlsx_workbook.h>
#include <xlsx_utils.h>
#include <cmath>
#include <algorithm>
namespace xlsx_reader {
    using namespace std;

    worksheet::worksheet(const workbook* in_workbook, uint32_t in_sheet_id, string_view in_sheet_name)
    : _workbook(in_workbook), _sheetId(in_sheet_id), _name(in_sheet_name)
    {
        auto the_raw_content = _workbook->get_sheet_raw_content(in_sheet_id);
        load_worksheet_from_string(the_raw_content);
    }
    void worksheet::load_worksheet_from_string(string_view input_string)
    {
        auto all_cells = load_cells_from_string(input_string, _workbook->shared_string);
        max_rows = 0;
        max_columns = 0;
        for(const auto& one_row : all_cells)
        {
            map<uint32_t, cell*> row_result;
            uint32_t current_row_id = one_row[0]->get_row();
            max_rows = max(current_row_id, max_rows);
            for(auto one_cell: one_row)
            {
                uint32_t current_column_id = one_cell->get_column();
                row_result[current_column_id] = one_cell;
                max_columns = max(max_columns, current_column_id);
            }
            _cells[current_row_id] = row_result;
        }
    }
    const map<uint32_t, cell*>& worksheet::get_row(uint32_t row_idx) const
    {
        return _cells.find(row_idx)->second;
    }
    const cell* worksheet::get_cell(uint32_t row_idx, uint32_t column_idx) const
    {
        auto row_iter = _cells.find(row_idx);
        if(row_iter == _cells.end())
        {
            return nullptr;
        }
        auto column_iter = row_iter->second.find(column_idx);
        if(column_iter == row_iter->second.end())
        {
            return nullptr;
        }
        else
        {
            return column_iter->second;
        }
    }
    uint32_t worksheet::get_max_row() const
    {
        return max_rows;
    }
    uint32_t worksheet::get_max_column() const
    {
        return max_columns;
    }
	string_view worksheet::get_name() const
	{
		return _name;
	}
    ostream& operator<<(ostream& output_stream, const worksheet& in_worksheet)
    {
        output_stream<<"worksheet name: "<< in_worksheet.get_name()<<", sheet_id: "<<in_worksheet._sheetId<<endl;
        for(const auto& one_row: in_worksheet._cells)
        {
            output_stream<<"row "<<one_row.first<<" has cells "<< one_row.second.size()<<endl;
            for(const auto& one_cell: one_row.second)
            {
                output_stream<<*(one_cell.second)<<endl;
            }
        }
		return output_stream;
    }
};