#include <xlsx_worksheet.h>
#include <xlsx_workbook.h>
#include <xlsx_utils.h>
#include <cmath>
#include <algorithm>
#include <xlsx_cell.h>
#include <iostream>

namespace xlsx_reader {
	using namespace std;

	worksheet::worksheet(const vector<cell>& in_all_cells, uint32_t in_sheet_id, string_view in_sheet_name)
	: _cells(in_all_cells), _sheet_id(in_sheet_id), _name(in_sheet_name)
	{
		load_from_cells();
	}

	void worksheet::load_from_cells()
	{
		row_info.clear();
		max_rows = 0;
		max_columns = 0;
		for(const auto& one_cell : _cells)
		{
			uint32_t current_row_id = one_cell.get_row();
			max_rows = max(current_row_id, max_rows);
			uint32_t current_column_id = one_cell.get_column();
			max_columns = max(max_columns, current_column_id);
			auto& cur_row_info = row_info[current_row_id];
			cur_row_info[current_column_id] = &one_cell;
		}
		after_load_process();
	}
	const map<uint32_t, const cell*>& worksheet::get_row(uint32_t row_idx) const
	{
		return row_info.find(row_idx)->second;
	}
	const cell* worksheet::get_cell(uint32_t row_idx, uint32_t column_idx) const
	{
		auto row_iter = row_info.find(row_idx);
		if(row_iter == row_info.end())
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
		output_stream<<"worksheet name: "<< in_worksheet.get_name()<<", sheet_id: "<<in_worksheet._sheet_id<<endl;
		for(const auto& one_row: in_worksheet.row_info)
		{
			output_stream<<"row "<<one_row.first<<" has cells "<< one_row.second.size()<<endl;
			for(const auto& one_cell: one_row.second)
			{
				output_stream<<*(one_cell.second)<<endl;
			}
		}
		return output_stream;
	}
	void worksheet::after_load_process()
	{
		cout<<"load complete for sheet "<< _name<<" with "<< max_rows<<" rows "<<max_columns<<" columns"<<endl;
	}
	worksheet::~worksheet()
	{

	}
	void to_json(json& j, const worksheet& cur_worksheet)
	{
		json new_j;
        json row_matrix;
		new_j["sheet_id"] = cur_worksheet._sheet_id;
		new_j["sheet_name"] = cur_worksheet._name;
        for(const auto& row_info: cur_worksheet.row_info)
        {
            auto cur_row_index = row_info.first;
            json row_j = json::object();
            for(const auto& column_info: row_info.second)
            {
                row_j[column_info.first] = json(*column_info.second);
            }
            row_matrix[cur_row_index] = row_j;
        }
        new_j["matrix"] = row_matrix;
        j = new_j;
        return;
	}
};