#include <xlsx_worksheet.h>
#include <xlsx_workbook.h>
#include <xlsx_utils.h>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace spiritsaway::xlsx_reader {
	using namespace std;

	worksheet::worksheet(const vector<cell>& in_all_cells, uint32_t in_sheet_id, const std::string& in_sheet_name, const workbook<worksheet>* in_workbook)
	: m_cells(in_all_cells), m_sheet_id(in_sheet_id), m_name(in_sheet_name), m_workbook(reinterpret_cast<const void*>(in_workbook))
	{
		load_from_cells();
	}

	void worksheet::load_from_cells()
	{
		row_info.clear();
		max_rows = 0;
		max_columns = 0;
		for(const auto& one_cell : m_cells)
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
		for (const auto& one_cell : m_cells)
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
	std::uint32_t worksheet::get_cell_shared_string_idx(std::uint32_t row_idx, std::uint32_t column_idx) const
	{
		if (row_idx > max_rows)
		{
			return std::numeric_limits<std::uint32_t>::max();
		}
		if (column_idx > max_columns)
		{
			return std::numeric_limits<std::uint32_t>::max();
		}

		auto ss_idx = row_info[row_idx][column_idx];
		return ss_idx;
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
	const std::string& worksheet::get_name() const
	{
		return m_name;
	}
	ostream& operator<<(ostream& output_stream, const worksheet& in_worksheet)
	{
		output_stream<<"worksheet name: "<< in_worksheet.get_name()<<", sheet_id: "<<in_worksheet.m_sheet_id<<endl;
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
	std::string worksheet::after_load_process()
	{
		// cout<<"load complete for sheet "<< _name<<" with "<< max_rows<<" rows "<<max_columns<<" columns"<<endl;
		return {};
	}
	worksheet::~worksheet()
	{

	}
	const workbook<worksheet>* worksheet::get_workbook() const
	{
		return reinterpret_cast<const workbook<worksheet>*>(m_workbook);
	}
};