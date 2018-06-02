#include <xlsx_worksheet.h>
#include <xlsx_workbook.h>
#include <xlsx_utils.h>
#include <cmath>
#include <algorithm>
#include <xlsx_cell.h>
namespace {
using namespace std;
using namespace xlsx_reader;
using namespace tinyxml2;

vector<cell> load_cells_from_xml(const XMLDocument* input_doc, const vector<string_view>& shared_string_table)
{
	auto& sheet_doc = *input_doc;
	vector<cell> result;
	auto worksheet_node = sheet_doc.FirstChildElement("worksheet");
	auto sheet_data_node = worksheet_node->FirstChildElement("sheetData");
	auto row_node = sheet_data_node->FirstChildElement("row");
	while(row_node)
	{
		uint32_t row_index = stoi(row_node->Attribute("r"));
		auto cell_node = row_node->FirstChildElement("c");
		while(cell_node)
		{
			uint32_t col_idx = row_column_tuple_from_string(cell_node->Attribute("r")).second;
			auto cur_cell = cell(row_index, col_idx);
			auto current_value = cell_node->FirstChildElement("v")->GetText();
			auto type_attr = cell_node->Attribute("t");
			if(type_attr)
			{
				auto type_attr_v = string(type_attr);
				if(type_attr_v == "s")
				{
					// shared str
					cur_cell.set_value(shared_string_table[stoi(current_value)]);
				}
				else if(type_attr_v == "str")
				{
					// simple_str
					cur_cell.set_value(current_value);
				}
				else if(type_attr_v == "b")
				{
					// BOOL
					auto bool_value = stoi(current_value) == 1;
					cur_cell.set_value(bool_value);
				}
				else if(type_attr_v == "n")
				{
					//numeric
					auto double_value = stod(current_value);
					cur_cell.set_value(double_value);
				}
				else if(type_attr_v == "inlineStr")
				{
					// simple_str
					cur_cell.set_value(current_value);
				}
				else if(type_attr_v == "e")
				{
					// error
					cur_cell.from_error(current_value);
				}
				else
				{
					// simple_str
					cur_cell.set_value(current_value);
				}
			}
			else
			{
				//numeric
				auto double_value = stod(current_value);
				cur_cell.set_value(double_value);
			}

			cell_node = cell_node->NextSiblingElement("c");
			result.push_back(cur_cell);

		}
		row_node = row_node->NextSiblingElement("row");
	}
	return result;
}
}
namespace xlsx_reader {
	using namespace std;

	worksheet::worksheet(const workbook* in_workbook, uint32_t in_sheet_id, string_view in_sheet_name)
	: _workbook(in_workbook), _sheetId(in_sheet_id), _name(in_sheet_name)
	{
		auto the_raw_content = _workbook->get_sheet_xml(in_sheet_id);
		_cells = load_cells_from_xml(the_raw_content, _workbook->shared_string);
		row_info.clear();
		max_rows = 0;
		max_columns = 0;
		for(const auto& one_cell : _cells)
		{
			uint32_t current_row_id = one_cell.get_row();
			max_rows = max(current_row_id, max_rows);
			uint32_t current_column_id = one_cell.get_column();
			max_columns = max(max_columns, current_column_id);
			auto cur_row_info = row_info[current_row_id];
			cur_row_info[current_column_id] = &one_cell;
		}
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
		output_stream<<"worksheet name: "<< in_worksheet.get_name()<<", sheet_id: "<<in_worksheet._sheetId<<endl;
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
};