#pragma once
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "xlsx_typed_cell.h"
#include "xlsx_workbook.h"
#include "xlsx_worksheet.h"
#include "xlsx_typed_worksheet.h"
namespace spiritsaway::xlsx_reader
{
	using json = nlohmann::json;
	using namespace std;
	void to_json(json& output, const typed_header& cur_typed_header)
	{
		json new_j;
		new_j["name"] = cur_typed_header.header_name;
		if (cur_typed_header.type_desc)
		{
			new_j["type_desc"] = cur_typed_header.type_desc->encode();
		}
		else
		{
			new_j["type_desc"] = json();
		}
		
		new_j["comment"] = cur_typed_header.header_comment;
		output = new_j;
		return;
	}
	void to_json(json& output, const typed_worksheet& cur_worksheet)
	{
		json new_j;
		json header_array = json::array();
		const auto& typed_headers = cur_worksheet.get_typed_headers();
		for (std::uint32_t i = 1; i < typed_headers.size(); i++)
		{
			header_array.push_back(json(typed_headers[i]));
		}

		new_j["headers"] = header_array;
		new_j["sheet_id"] = cur_worksheet.m_sheet_id;
		new_j["sheet_name"] = cur_worksheet.m_name;
		json row_matrix;
		for (std::uint32_t i = cur_worksheet.value_begin_row(); i <= cur_worksheet.get_max_row(); i++)
		{
			json row_j = json::object();
			for (std::uint32_t j = 1; j <= cur_worksheet.get_max_column(); j++)
			{
				const auto& cur_cell_json = cur_worksheet.get_typed_cell_value(i, j);
				if (!cur_cell_json.is_null())
				{
					row_j[std::string(typed_headers[j].header_name)] = cur_cell_json;
				}
			}
			row_matrix.push_back(row_j);
		}
		

		new_j["matrix"] = row_matrix;
		output = new_j;
		return;
	}
	template <typename T>
	void to_json(json& j, const workbook<T>& in_workbook)
	{
		json result;
		result["sheet_relation"] = in_workbook.sheet_relations;
		json all_sheets ;
		for (const auto& i : in_workbook.m_worksheets)
		{
			all_sheets[string(i->m_name)] = *i;
		}
		result["sheets"] = all_sheets;
		result["name"] = in_workbook.get_workbook_name();
		j = result;
		return;
	}
	void to_json(json& j, const worksheet& cur_worksheet)
	{
		json new_j;
		json row_matrix;
		new_j["sheet_id"] = cur_worksheet.m_sheet_id;
		new_j["sheet_name"] = cur_worksheet.m_name;
		const auto& all_row_info = cur_worksheet.get_all_row();
		for (std::uint32_t i = 0; i < all_row_info.size(); i++)
		{
			auto cur_row_index = i;
			json row_j = json::object();
			for (std::uint32_t j = 0; j < all_row_info[i].size(); j++)
			{
				row_j[j] = cur_worksheet.get_cell(i, j);
			}
			row_matrix[cur_row_index] = row_j;
		}
		new_j["matrix"] = row_matrix;
		j = new_j;
		return;
	}
	
	
}
