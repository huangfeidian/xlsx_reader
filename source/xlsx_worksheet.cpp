#include <xlsx_worksheet.h>
#include <xlsx_workbook.h>
#include <xlsx_utils.h>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace spiritsaway::xlsx_reader {
	using namespace std;

	worksheet::worksheet(const vector<cell>& in_all_cells, uint32_t in_sheet_id, string_view in_sheet_name, const workbook<worksheet>* in_workbook)
	: _cells(in_all_cells), _sheet_id(in_sheet_id), _name(in_sheet_name), _workbook(reinterpret_cast<const void*>(in_workbook))
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
			uint32_t current_row_id = get<0>(one_cell);
			max_rows = max(current_row_id, max_rows);
			uint32_t current_column_id = get<1>(one_cell);
			max_columns = max(max_columns, current_column_id);
		}
		row_info.reserve(max_rows + 1);
		row_info.emplace_back();
		for (std::size_t i = 0; i < max_rows; i++)
		{
			row_info.emplace_back(max_columns + 1);
		}
		for (const auto& one_cell : _cells)
		{
			uint32_t current_row_id = get<0>(one_cell);
			uint32_t current_column_id = get<1>(one_cell);
			uint32_t current_ss_idx = get<2>(one_cell);
			row_info[current_row_id][current_column_id] = current_ss_idx;
		}
		after_load_process();
	}
	const vector<uint32_t>& worksheet::get_row(uint32_t row_idx) const
	{
		return row_info[row_idx];
	}
	const vector<vector<uint32_t>>& worksheet::get_all_row()const
	{
		return row_info;
	}
	string_view worksheet::get_cell(uint32_t row_idx, uint32_t column_idx) const
	{
		if (row_idx > max_rows)
		{
			return string_view();
		}
		if (column_idx > max_columns)
		{
			return string_view();
		}

		auto ss_idx = row_info[row_idx][column_idx];
		auto the_workbook = get_workbook();
		return the_workbook->get_shared_string(ss_idx);
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
		const auto& all_row_info = in_worksheet.get_all_row();
		for (std::uint32_t i = 1; i < all_row_info.size(); i++)
		{
			output_stream << "row " << i << " has cells " << all_row_info[i].size() - 1 << endl;
			for (std::uint32_t j = 1; j < all_row_info[i].size(); j++)
			{
				output_stream << "cell at row " << i << " column " << j << " with value " << in_worksheet.get_cell(i, j) << endl;
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
	const workbook<worksheet>* worksheet::get_workbook() const
	{
		return reinterpret_cast<const workbook<worksheet>*>(_workbook);
	}
	uint32_t worksheet::memory_consumption() const
	{
		uint32_t result = sizeof(uint32_t) * max_columns * max_rows;
		return result;
	}
};