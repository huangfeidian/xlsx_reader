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
		typed_headers.clear();
		typed_headers.push_back(nullptr);
		int column_idx = 1;
		if (get_max_row() < 1)
		{
			return false;
		}
		const auto& header_name_row = get_row(1);
		if (header_name_row.empty())
		{
			return false;
		}
		for(int i= 1; i< header_name_row.size(); i++)
		{			
			auto cur_header_name = get_cell(1, i);
			if(cur_header_name.empty())
			{
				cerr<<"empty header name at idx "<<i<<endl;
				return false;
			}

			auto cur_cell_value = get_cell(2, column_idx);

			if (cur_cell_value.empty())
			{
				cerr <<"invalid type desc for header type at column " << i << endl;
			}
			auto cur_type_desc = typed_value_parser::parse_type(cur_cell_value);
			string_view header_comment = get_cell(3, column_idx);
			typed_headers.push_back(new typed_header(cur_type_desc, cur_header_name, header_comment));

			if (column_idx == 1)
			{
				// expect int or str in first column
				switch (cur_type_desc->_type)
				{
				case basic_value_type::number_bool:
				case basic_value_type::number_float:
				case basic_value_type::number_double:
				case basic_value_type::comment:
				case basic_value_type::list:
				case basic_value_type::tuple:
					cerr << "first column value type should be int or string" << endl;
					return false;
				default:
					break;
				}
			}
			if (get_header_idx(cur_header_name) != 0)
			{
				cerr << "duplicated header name " << cur_header_name << endl;
			}
			header_column_index[cur_header_name] = column_idx;
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
		all_cell_values.clear();
		auto value_begin_row_idx = value_begin_row();
		const auto& all_row_info = get_all_row();
		if (value_begin_row_idx > max_rows)
		{
			all_cell_values.emplace_back();
			return;
		}
		all_cell_values.reserve(1 + max_rows - value_begin_row_idx);
		all_cell_values.emplace_back();
		for (int i = value_begin_row_idx; i < all_row_info.size(); i++)
		{
			all_cell_values.emplace_back(all_row_info[i].size());
		}
		for (int i = value_begin_row(); i < all_row_info.size(); i++)
		{
			for (int j = 1; j < all_row_info[i].size(); j++)
			{
				string_view cur_cell = get_cell(i, j);
				if (cur_cell.empty())
				{
					continue;
				}
				all_cell_values[i - value_begin_row_idx + 1][j].~typed_value();
				typed_value_parser::parse_value_with_type(typed_headers[j]->type_desc, cur_cell, all_cell_values[i - value_begin_row_idx + 1][j]);
			}
			if (all_cell_values[i - value_begin_row_idx + 1][1].type_desc)
			{
				_indexes[&(all_cell_values[i - value_begin_row_idx + 1][1])] = i - value_begin_row_idx + 1;
			}
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
		if (convert_typed_header())
		{
			convert_cell_to_typed_value();
		}
		else
		{
			for (auto i : typed_headers)
			{
				if (i)
				{
					delete i;
				}
			}
			typed_headers.clear();
			header_column_index.clear();
		}
		
	}
	typed_worksheet::~typed_worksheet()
	{
		for (auto i : typed_headers)
		{
			if (i)
			{
				delete i;
			}
		}
		for (auto& i : all_cell_values)
		{
			for (auto& j : i)
			{
				j.cleaup_recursive();
			}
		}

	}
	const vector<const typed_header*>& typed_worksheet::get_typed_headers() const
	{
		return typed_headers;
	}
	uint32_t typed_worksheet::get_header_idx(string_view header_name) const
	{
		auto header_iter = header_column_index.find(header_name);
		if (header_iter == header_column_index.end())
		{
			return 0;
		}
		else
		{
			return header_iter->second;
		}
	}
	uint32_t typed_worksheet::get_indexed_row(const typed_value* first_row_value) const
	{
		auto iter = _indexes.find(first_row_value);
		if(iter == _indexes.end())
		{
			return 0;
		}
		else
		{
			return iter->second;
		}
	}
	const typed_value* typed_worksheet::get_typed_cell_value(uint32_t row_idx, uint32_t column_idx) const
	{
		if (row_idx == 0 || column_idx == 0)
		{
			return nullptr;
		}
		if (row_idx >= all_cell_values.size())
		{
			return nullptr;
		}
		if (column_idx >= all_cell_values[row_idx].size())
		{
			return nullptr;
		}
		return &all_cell_values[row_idx][column_idx];
	}
	const vector<typed_value>& typed_worksheet::get_ref_row(string_view sheet_name, const typed_value*  first_row_value) const
	{
		auto current_workbook = get_workbook();
		if(! current_workbook)
		{
			return all_cell_values[0];
		}
		auto sheet_idx = current_workbook->get_sheet_index_by_name(sheet_name);
		if(! sheet_idx)
		{
			return all_cell_values[0];
		}
		const auto& the_worksheet = current_workbook->get_worksheet(sheet_idx.value());
		auto row_index = the_worksheet.get_indexed_row(first_row_value);
		if(!row_index)
		{
			return all_cell_values[0];
		}
		return the_worksheet.get_typed_row(row_index);
	}
	const vector<typed_value>& typed_worksheet::get_typed_row(uint32_t _idx) const
	{
		if (_idx == 0 || _idx >= all_cell_values.size())
		{
			return all_cell_values[0];
		}
		else
		{
			return all_cell_values[_idx];
		}
	}
	const vector<vector<typed_value>>& typed_worksheet::get_all_typed_row_info() const
	{
		return all_cell_values;
	}
	bool typed_worksheet::check_header_match(const unordered_map<string_view, const typed_header*>& other_headers, string_view index_column_name, const vector<string_view>& int_ref_headers, const vector<string_view>& string_ref_headers) const
	{
		if (typed_headers.size() < 2)
		{
			cout << "current sheet doesnt has headers " << endl;
			return false;
		}
		if(typed_headers[1]->header_name != index_column_name)
		{
			cout << "index column name mismatch input " << index_column_name << " current " << typed_headers[1]->header_name << endl;
			return false;
		}
		for(const auto& i : other_headers)
		{
			if (!i.second)
			{
				cout << "missing type desc for header " << i.first << endl;
				return false;
			}
			auto header_idx = get_header_idx(i.second->header_name);
			if(header_idx == 0)
			{
				cout << " cant find header " << i.second->header_name << endl;
				return false;
			}
			header_idx--;
			if (header_idx >= typed_headers.size())
			{
				cout << " cant find header " << i.second->header_name << endl;
				return false;
			}
			if (!typed_headers[header_idx])
			{
				cout << " cant find header " << i.second->header_name << endl;
				return false;
			}
			if(!(*typed_headers[header_idx] == *i.second))
			{
				cout << "header type mismatch for  " << i.second->header_name << endl;
				return false;
			}

		}
		for(auto int_ref_name: int_ref_headers)
		{
			auto header_idx = get_header_idx(int_ref_name);
			if(header_idx == 0)
			{
				cout << "cant find ref int header " << int_ref_name << endl;
				return false;
			}
			auto cur_header = typed_headers[header_idx];

			if(!cur_header ||!cur_header->type_desc)
			{
				cout << "type desc is empty for ref int header " << int_ref_name << endl;
				return false;
			}
			if(cur_header->type_desc->_type != basic_value_type::ref_id)
			{
				cout << " not ref type for ref int header " << int_ref_name << endl;
				return false;
			}
			auto ref_detail = cur_header->type_desc->get_ref_detail_t();
			if(!ref_detail)
			{
				cout << " not ref type for ref int header " << int_ref_name << endl;
				return false;
			}
			if(std::get<2>(ref_detail.value()) != "int"sv)
			{
				cout << " not int ref type for ref int header " << int_ref_name << endl;
				return false;
			}
		}
		for(auto str_ref_name: string_ref_headers)
		{
			auto header_idx = get_header_idx(str_ref_name);
			if(header_idx == 0)
			{
				cout << "cant find ref str header " << str_ref_name << endl;
				return false;
			}
			header_idx--;
			auto cur_header = typed_headers[header_idx];
			if(!cur_header || !cur_header->type_desc)
			{
				cout << "type desc is empty for ref str header " << str_ref_name << endl;
				return false;
			}
			if(cur_header->type_desc->_type != basic_value_type::ref_id)
			{
				cout << " not ref type for ref str header " << str_ref_name << endl;
				return false;
			}
			auto ref_detail = cur_header->type_desc->get_ref_detail_t();
			if(!ref_detail)
			{
				cout << " not ref type for ref str header " << str_ref_name << endl;
				return false;
			}
			if(std::get<2>(ref_detail.value()) != "str"sv)
			{
				cout << " not str ref type for ref str header " << str_ref_name << endl;
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
	std::uint32_t typed_worksheet::memory_details() const
	{
		std::uint32_t result = 0;
		uint32_t temp = 0;
		temp = sizeof(vector<typed_header*>) + typed_headers.capacity() * sizeof(typed_header*) + typed_headers.size() * sizeof(typed_header);
		auto parent_size = worksheet::memory_details();
		cout << "parent worksheet size is " << parent_size << endl;
		result += parent_size;
		cout << "typed headers size " << temp<<endl;
		result += temp;
		temp = 0;
		for (const auto& i : all_cell_values)
		{
			for (const auto& j : i)
			{
				temp += j.memory_details();
			}
			
		}
		cout << "typed_value memory " << temp << " with size " << (max_rows - value_begin_row()) * max_columns << endl;
		result += temp;
		temp = 0;
		temp += 12 * _indexes.bucket_count();
		cout << "_indexes memory " << temp << " with size " << _indexes.size() << endl;
		result += temp;

		cout << "sheet "  << _name << " memory total "<< result<<endl<<endl;
		return result;
	}
	vector<uint32_t> typed_worksheet::get_header_index_vector(const vector<string_view>& header_names) const
	{
		vector<uint32_t> result;
		for (const auto& i : header_names)
		{
			result.push_back(get_header_idx(i));
		}
		return result;
	}
}