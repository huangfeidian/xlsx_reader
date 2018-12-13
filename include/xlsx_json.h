#pragma once
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "xlsx_cell.h"
#include "xlsx_cell_extend.h"
#include "xlsx_workbook.h"
#include "xlsx_worksheet.h"
#include "xlsx_typed.h"
namespace xlsx_reader
{
	using json = nlohmann::json;
	using namespace std;
	void to_json(json& j, const cell& cur_cell)
	{
		j = string(cur_cell._text);
		return;
	}
	void to_json(json& j, const typed_header& cur_typed_header)
	{
		auto new_j = json({{"name", cur_typed_header.header_name}, {"type_desc", *cur_typed_header.type_desc}, {"comment", cur_typed_header.header_comment}});
		j = new_j;
		return;
	}
	void to_json(json& j, const typed_worksheet& cur_worksheet)
	{
		json new_j;
		json header_array = json::array();
		for (const auto& i : cur_worksheet.typed_headers)
		{
			header_array.push_back(json(*i));
		}
		new_j["headers"] = header_array;
		new_j["sheet_id"] = cur_worksheet._sheet_id;
		new_j["sheet_name"] = cur_worksheet._name;
		json row_matrix;

		for(const auto& row_info: cur_worksheet.typed_row_info)
		{
			auto cur_row_index = row_info.first;
			json row_j = json::object();
			for(const auto& column_info: row_info.second)
			{
				row_j[std::string((cur_worksheet.typed_headers[column_info.first - 1])->header_name)] = json(*column_info.second->cur_typed_value);
			}
			row_matrix[to_string(cur_row_index)] = row_j;
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
	void to_json(json& j, const extend_node_type_descriptor& cur_type)
	{
		static unordered_map<basic_node_type_descriptor, string_view> type_to_string = {
			{basic_node_type_descriptor::comment, "comment"},
			{basic_node_type_descriptor::date, "date"},
			{basic_node_type_descriptor::time, "time"},
			{basic_node_type_descriptor::datetime, "datetime"},
			{basic_node_type_descriptor::string, "string"},
			{basic_node_type_descriptor::number_bool, "bool"},
			{basic_node_type_descriptor::number_u32, "uint32"},
			{basic_node_type_descriptor::number_32, "int32"},
			{basic_node_type_descriptor::number_64, "int64"},
			{basic_node_type_descriptor::number_u64, "uint64"},
			{basic_node_type_descriptor::number_float, "float"},
			{basic_node_type_descriptor::number_double, "double"},
		};
		auto temp_iter = type_to_string.find(cur_type._type);
		if(temp_iter != type_to_string.end())
		{
			j = json(temp_iter->second);
			return;
		}
		
		switch(cur_type._type)
		{
		case basic_node_type_descriptor::list:
			{
				auto temp_detail = std::get<extend_node_type_descriptor::list_detail_t>(cur_type._type_detail);
				json result_json;
				string sep_string = ",";
				sep_string[0] = std::get<2>(temp_detail);
				result_json["list"] = {{"type", *std::get<0>(temp_detail)}, {"seperator", sep_string}, {"size", std::get<1>(temp_detail)}};
				j = result_json;
				return; 
			}
		case basic_node_type_descriptor::tuple:
			{
				auto temp_detail = std::get<extend_node_type_descriptor::tuple_detail_t>(cur_type._type_detail);
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
		case basic_node_type_descriptor::ref_id:
			{
				auto temp_detail = std::get<extend_node_type_descriptor::ref_detail_t>(cur_type._type_detail);
				string_view cur_workbook, cur_worksheet, cur_ref_type;
				if(!cur_workbook.empty())
				{
					json result_json;
					result_json["ref"] = {cur_worksheet, cur_ref_type};
					j = result_json;
					return;
				}
				else
				{
					json result_json;
					result_json["ref"] = {cur_workbook, cur_worksheet, cur_ref_type};
					j = result_json;
					return;
				}
			}
		default:
			j = nullptr;
			return;
		}
		return;
	}
	void to_json(json& j, const extend_node_value& cur_value)
	{
		if (!cur_value.type_desc)
		{
			j = nullptr;
			return;
		}
		switch(cur_value.type_desc->_type)
		{
		case basic_node_type_descriptor::comment:
			j = cur_value.v_text;
			return;
		case basic_node_type_descriptor::date:
			j = date::from_number(cur_value.v_int32, calendar::windows_1900).to_string();
			return;
		case basic_node_type_descriptor::time:
			j = time::from_number(cur_value.v_double).to_string();
			return;
		case basic_node_type_descriptor::number_bool:
			j = cur_value.v_bool;
			return;
		case basic_node_type_descriptor::number_32:
			j = cur_value.v_int32;
			return;
		case basic_node_type_descriptor::number_u32:
			j = cur_value.v_uint32;
			return;
		case basic_node_type_descriptor::number_64:
			j = cur_value.v_int64;
			return;

		case basic_node_type_descriptor::number_u64:
			j = cur_value.v_uint64;
			return;

		case basic_node_type_descriptor::number_float:
			j = cur_value.v_float;
			return;
		case basic_node_type_descriptor::number_double:
			j = cur_value.v_double;
			return;
		case basic_node_type_descriptor::ref_id:
			j = cur_value.v_text;
			return;
		case basic_node_type_descriptor::string:
			j = cur_value.v_text;
			return;
		case basic_node_type_descriptor::list:
			j = json::array();
			for(const auto& i: cur_value.v_list)
			{
				j.push_back(json(*i));
			}
			return;
		case basic_node_type_descriptor::tuple:
			j = json::array();
			for(const auto& i: cur_value.v_list)
			{
				j.push_back(json(*i));
			}
			return;
		default:
			j = nullptr;
			return;
		}
	}
}
