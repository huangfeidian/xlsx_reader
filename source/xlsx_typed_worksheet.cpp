#include <xlsx_typed_worksheet.h>
#include <iostream>
namespace {
	using namespace xlsx_reader;
	using namespace std;

}
namespace xlsx_reader{
	using namespace std;
	typed_header::typed_header(const typed_node_type_descriptor* in_type_desc, string_view in_header_name, string_view in_header_comment):type_desc(in_type_desc), header_name(in_header_name), header_comment(in_header_comment)
	{

	}
	std::ostream& operator<<(std::ostream& output_stream, const typed_header& in_typed_header)
	{
		output_stream<<"name: "<< in_typed_header.header_name<<" type_desc: "<<*in_typed_header.type_desc<<" comment:"<<in_typed_header.header_comment<<endl;
		return output_stream;
	}

	bool typed_worksheet::convert_typed_header()
	{
		int column_idx = 1;
		auto header_name_row = get_row(1);
		for(const auto& i: header_name_row)
		{
			if(i.first != column_idx)
			{
				cout<<"not continuous header row, current is "<< i.first<<" while expecting "<< column_idx<<endl;
				return false;
			}
			
			auto cur_cell_value = i.second;
			if(!cur_cell_value)
			{
				cerr<<"empty header name at idx "<<i.first<<endl;
				return false;
			}
			auto cur_header_name_opt = cur_cell_value->expect_value<string_view>();
			if (!cur_header_name_opt)
			{
				cerr << "empty header name at idx " << i.first << endl;
				return false;
			}
			auto cur_header_name = cur_header_name_opt.value();
			cur_cell_value = get_cell(2, column_idx);
			if(!cur_cell_value)
			{
				cerr<<"empty cell type value for header "<< cur_header_name<<endl;
				return false;
			}
			auto cur_type_opt = cur_cell_value->expect_value<string_view>();
			if (!cur_type_opt)
			{
				cerr <<"invalid type desc "<<cur_cell_value->_text<<" for header type at column " << i.first << endl;
			}
			auto cur_type_desc = typed_node_value_constructor::parse_type(cur_type_opt.value());

			if (column_idx == 1)
			{
				// expect int or str in first column
				switch (cur_type_desc->_type)
				{
				case basic_node_type_descriptor::number_bool:
				case basic_node_type_descriptor::number_float:
				case basic_node_type_descriptor::number_double:
				case basic_node_type_descriptor::comment:
				case basic_node_type_descriptor::list:
				case basic_node_type_descriptor::tuple:
					cerr << "first column value type should be int or string" << endl;
					return false;
				default:
					break;
				}
			}
			string_view header_comment;
			cur_cell_value = get_cell(3, column_idx);
			if(cur_cell_value)
			{
				header_comment = cur_cell_value->_text;

			}
			typed_headers.push_back(new typed_header(cur_type_desc, cur_header_name, header_comment));
			column_idx += 1;
		}
		return true;
	}
	int typed_worksheet::value_begin_row() const
	{
		return 4;
	}
	void typed_worksheet::convert_cell_to_typed_value()
	{
		typed_cells.clear();
		auto value_begin_row_idx = value_begin_row();
		for(const auto& i: row_info)
		{
			if(i.first < value_begin_row_idx)
			{
				continue;
			}
			map<std::uint32_t, const typed_cell*> cur_row_typed_info;
			for(const auto& j: i.second)
			{
				if(!j.second)
				{
					continue;
				}
				auto cur_typed_cell = typed_node_value_constructor::parse_node(typed_headers[j.first - 1]->type_desc, j.second);
				if(!cur_typed_cell)
				{
					continue;
				}
				cur_row_typed_info[j.first - 1] = cur_typed_cell;
				typed_cells.push_back(*cur_typed_cell);
			}
			typed_row_info[i.first - value_begin_row_idx] = cur_row_typed_info;
			_indexes[cur_row_typed_info.cbegin()->second->cur_typed_value] = i.first - value_begin_row_idx;

		}
	}
	typed_worksheet::typed_worksheet(const vector<cell>& all_cells, uint32_t in_sheet_id, string_view in_sheet_name, const workbook<typed_worksheet>* in_workbook)
	: worksheet(all_cells, in_sheet_id, in_sheet_name, reinterpret_cast<const workbook<worksheet>*>(in_workbook))
	{

	}
	const workbook<typed_worksheet>* typed_worksheet::get_workbook() const
	{
		return reinterpret_cast<const workbook<typed_worksheet>*>(_workbook);
	}
	void typed_worksheet::after_load_process()
	{
		convert_typed_header();
		convert_cell_to_typed_value();
	}
	typed_worksheet::~typed_worksheet()
	{

	}
	const vector<const typed_header*>& typed_worksheet::get_typed_headers() const
	{
		return typed_headers;
	}
	uint32_t typed_worksheet::get_header_idx(string_view header_name) const
	{
		for (int i = 0; i < typed_headers.size(); i++)
		{
			if (typed_headers[i]->header_name == header_name)
			{
				return i + 1;
			}
		}
		return 0;
	}
	optional<uint32_t> typed_worksheet::get_indexed_row(const typed_value* first_row_value) const
	{
		auto iter = _indexes.find(first_row_value);
		if(iter == _indexes.end())
		{
			return nullopt;
		}
		else
		{
			return iter->second;
		}
	}
	const typed_cell* typed_worksheet::get_typed_cell(uint32_t row_idx, uint32_t column_idx) const
	{
		auto row_iter = typed_row_info.find(row_idx);
		if (row_iter == typed_row_info.end())
		{
			return nullptr;
		}
		auto the_row = row_iter->second;
		auto column_iter = the_row.find(column_idx);
		if (column_iter == the_row.end())
		{
			return nullptr;
		}
		return column_iter->second;
	}
	optional<reference_wrapper<const map<uint32_t, const typed_cell*>>> typed_worksheet::get_ref_row(string_view sheet_name, const typed_value*  first_row_value) const
	{
		auto current_workbook = get_workbook();
		if(! current_workbook)
		{
			return nullopt;
		}
		auto sheet_idx = current_workbook->get_sheet_index_by_name(sheet_name);
		if(! sheet_idx)
		{
			return nullopt;
		}
		auto the_worksheet = current_workbook->get_worksheet(sheet_idx.value());
		auto row_index = the_worksheet.get_indexed_row(first_row_value);
		if(!row_index)
		{
			return nullopt;
		}
		return cref(the_worksheet.get_typed_row(row_index.value()));
	}
	const map<uint32_t, const typed_cell*>& typed_worksheet::get_typed_row(uint32_t _idx) const
	{
		return typed_row_info.find(_idx)->second;
	}
	const std::map<std::uint32_t, std::map<std::uint32_t, const typed_cell*>>& typed_worksheet::get_all_typed_row_info() const
	{
		return typed_row_info;
	}
	bool typed_worksheet::check_header_match(const unordered_map<string_view, const typed_header*>& other_headers, string_view index_column_name, const vector<string_view>& int_ref_headers, const vector<string_view>& string_ref_headers) const
	{
		if(typed_headers[0]->header_name != index_column_name)
		{
			return false;
		}
		for(const auto& i : other_headers)
		{
			if (!i.second)
			{
				return false;
			}
			auto header_idx = get_header_idx(i.second->header_name);
			if(header_idx == 0)
			{
				return false;
			}
			header_idx--;
			if (header_idx >= typed_headers.size())
			{
				return false;
			}
			if (!typed_headers[header_idx])
			{
				return false;
			}
			if(!(*typed_headers[header_idx] == *i.second))
			{
				return false;
			}

		}
		for(auto inf_ref_name: int_ref_headers)
		{
			auto header_idx = get_header_idx(inf_ref_name);
			if(header_idx == 0)
			{
				return false;
			}
			auto cur_header = typed_headers[header_idx];

			if(!cur_header ||!cur_header->type_desc)
			{
				return false;
			}
			if(cur_header->type_desc->_type != basic_node_type_descriptor::ref_id)
			{
				return false;
			}
			auto ref_detail = cur_header->type_desc->get_ref_detail_t();
			if(!ref_detail)
			{
				return false;
			}
			if(std::get<2>(ref_detail.value()) != "int"sv)
			{
				return false;
			}
		}
		for(auto inf_ref_name: string_ref_headers)
		{
			auto header_idx = get_header_idx(inf_ref_name);
			if(header_idx == 0)
			{
				return false;
			}
			header_idx--;
			auto cur_header = typed_headers[header_idx];
			if(!cur_header || !cur_header->type_desc)
			{
				return false;
			}
			if(cur_header->type_desc->_type != basic_node_type_descriptor::ref_id)
			{
				return false;
			}
			auto ref_detail = cur_header->type_desc->get_ref_detail_t();
			if(!ref_detail)
			{
				return false;
			}
			if(std::get<2>(ref_detail.value()) != "str"sv)
			{
				return false;
			}
		}
		return true;
	}
	bool typed_header::operator==(const typed_header& other) const
	{
		if(header_name != other.header_name)
		{
			return false;
		}
		if(type_desc == other.type_desc)
		{
			return true;
		}
		if(!type_desc || !other.type_desc)
		{
			return false;
		}
		return *type_desc == *other.type_desc;

	}
}