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
	void to_json(json& j, const typed_node_type_descriptor& cur_type)
	{
		static unordered_map<basic_value_type, string_view> type_to_string = {
			{basic_value_type::comment, "comment"},
			{basic_value_type::string, "string"},
			{basic_value_type::number_bool, "bool"},
			{basic_value_type::number_u32, "uint32"},
			{basic_value_type::number_32, "int32"},
			{basic_value_type::number_64, "int64"},
			{basic_value_type::number_u64, "uint64"},
			{basic_value_type::number_float, "float"},
			{basic_value_type::number_double, "double"},
		};
		auto temp_iter = type_to_string.find(cur_type._type);
		if (temp_iter != type_to_string.end())
		{
			j = json(temp_iter->second);
			return;
		}

		switch (cur_type._type)
		{
		case basic_value_type::list:
		{
			auto temp_detail = std::get<typed_node_type_descriptor::list_detail_t>(cur_type._type_detail);
			json result_json;
			string sep_string = ",";
			sep_string[0] = std::get<2>(temp_detail);
			result_json["list"] = { {"type", *std::get<0>(temp_detail)}, {"seperator", sep_string}, {"size", std::get<1>(temp_detail)} };
			j = result_json;
			return;
		}
		case basic_value_type::tuple:
		{
			auto temp_detail = std::get<typed_node_type_descriptor::tuple_detail_t>(cur_type._type_detail);
			json result_json;
			json type_detail = json::array();
			for (const auto& i : temp_detail.first)
			{
				type_detail.push_back(*i);
			}
			string sep_string = ",";
			sep_string[0] = temp_detail.second;
			result_json["tuple"] = { {"type", type_detail}, {"seperator", sep_string} };
			j = result_json;
			return;
		}
		case basic_value_type::ref_id:
		{
			auto temp_detail = std::get<typed_node_type_descriptor::ref_detail_t>(cur_type._type_detail);
			auto [cur_worksheet, cur_ref_type] = templ_detail;
			json result_json;
			result_json["ref"] = { cur_worksheet, cur_ref_type };
			j = result_json;
			return;
			
		}
		default:
			j = nullptr;
			return;
		}
		return;
	}
	void to_json(json& j, const typed_value& cur_value)
	{
		if (!cur_value.type_desc)
		{
			j = nullptr;
			return;
		}
		switch (cur_value.type_desc->_type)
		{
		case basic_value_type::comment:
			j = cur_value.v_text;
			return;
		case basic_value_type::number_bool:
			j = cur_value.v_bool;
			return;
		case basic_value_type::number_32:
			j = cur_value.v_int32;
			return;
		case basic_value_type::number_u32:
			j = cur_value.v_uint32;
			return;
		case basic_value_type::number_64:
			j = cur_value.v_int64;
			return;

		case basic_value_type::number_u64:
			j = cur_value.v_uint64;
			return;

		case basic_value_type::number_float:
			j = cur_value.v_float;
			return;
		case basic_value_type::number_double:
			j = cur_value.v_double;
			return;
		case basic_value_type::ref_id:
			j = cur_value.v_text;
			return;
		case basic_value_type::string:
			j = cur_value.v_text;
			return;
		case basic_value_type::list:
			j = json::array();
			for (const auto& i : cur_value.v_list)
			{
				j.push_back(json(*i));
			}
			return;
		case basic_value_type::tuple:
			j = json::array();
			for (const auto& i : cur_value.v_list)
			{
				j.push_back(json(*i));
			}
			return;
		default:
			j = nullptr;
			return;
		}
	}
	void to_json(json& j, const typed_header& cur_typed_header)
	{
		json new_j;
		new_j["name"] = cur_typed_header.header_name;
		new_j["type_desc"] = json(*cur_typed_header.type_desc);
		new_j["comment"] = cur_typed_header.header_comment;
		j = new_j;
		return;
	}
	void to_json(json& j, const typed_worksheet& cur_worksheet)
	{
		json new_j;
		json header_array = json::array();
		const auto& typed_headers = cur_worksheet.get_typed_headers();
		for (int i = 1; i < typed_headers.size(); i++)
		{
			header_array.push_back(json(*typed_headers[i]));
		}

		new_j["headers"] = header_array;
		new_j["sheet_id"] = cur_worksheet._sheet_id;
		new_j["sheet_name"] = cur_worksheet._name;
		json row_matrix;
		const auto& all_row_info = cur_worksheet.get_all_typed_row_info();
		
		for (int i = 1; i < all_row_info.size(); i++)
		{
			auto cur_row_index = i;
			json row_j = json::object();
			for (int j = 1; j < all_row_info[i].size(); j++)
			{
				row_j[std::string((typed_headers[j])->header_name)] = json(all_row_info[i][j]);
			}
			row_matrix[to_string(cur_row_index)] = row_j;
		}
		for(const auto& row_info: cur_worksheet.get_all_typed_row_info())
		{
			
		}
		new_j["matrix"] = row_matrix;
		j = new_j;
		return;
	}
	template <typename T>
	void to_json(json& j, const workbook<T>& in_workbook)
	{
		json result;
		result["sheet_relation"] = in_workbook.sheet_relations;
		json all_sheets ;
		for (const auto& i : in_workbook._worksheets)
		{
			all_sheets[string(i->_name)] = *i;
		}
		result["sheets"] = all_sheets;
		result["name"] = in_workbook.workbook_name;
		j = result;
		return;
	}
	void to_json(json& j, const worksheet& cur_worksheet)
	{
		json new_j;
		json row_matrix;
		new_j["sheet_id"] = cur_worksheet._sheet_id;
		new_j["sheet_name"] = cur_worksheet._name;
		const auto& all_row_info = cur_worksheet.get_all_row();
		for (int i = 0; i < all_row_info.size(); i++)
		{
			auto cur_row_index = i;
			json row_j = json::object();
			for (int j = 0; j < all_row_info[i].size(); j++)
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
