#include <xlsx_typed_worksheet.h>
#include <iostream>

#include <sstream>

namespace {
	using namespace spiritsaway::xlsx_reader;
	using namespace std;

}
namespace spiritsaway::xlsx_reader{
	using namespace std;
	typed_header::typed_header(std::shared_ptr<const typed_string_desc> in_type_desc, string_view in_header_name, string_view in_header_comment)
		:type_desc(in_type_desc), header_name(in_header_name), header_comment(in_header_comment)
	{

	}
	std::ostream& operator<<(std::ostream& output_stream, const typed_header& in_typed_header)
	{
		output_stream<<"name: "<< in_typed_header.header_name<<" type_desc: "<<*in_typed_header.type_desc<<" comment:"<<in_typed_header.header_comment<<endl;
		return output_stream;
	}

	
	std::string typed_worksheet::convert_typed_header()
	{
		m_typed_headers.clear();
		m_typed_headers.push_back(typed_header( {}, {}, {}));
		int column_idx = 1;
		if (get_max_row() < 1)
		{
			return "invalid max row";
		}
		const auto& header_name_row = get_row(1);
		if (header_name_row.empty())
		{
			return "header name row empty";
		}
		std::ostringstream oss;
		for(std::size_t i= 1; i< header_name_row.size(); i++)
		{			
			auto cur_header_name = get_cell(1, i);
			if(cur_header_name.empty())
			{
				oss <<"empty header name at idx "<<i<<endl;
				return oss.str();
			}

			auto cur_cell_value = get_cell(2, column_idx);

			if (cur_cell_value.empty())
			{
				oss <<"empty type desc for header type at column " << i << endl;
				return oss.str();
			}
			auto cur_type_desc = typed_string_desc::get_type_from_str(cur_cell_value);
			if (!cur_type_desc)
			{
				oss << "invalid type desc "<< cur_cell_value<<" for header type at column " << i << endl;
				return oss.str();
			}
			string_view header_comment = get_cell(3, column_idx);
			m_typed_headers.push_back(typed_header(cur_type_desc, cur_header_name, header_comment));

			if (column_idx == 1)
			{
				// expect int or str in first column
				switch (cur_type_desc->m_type)
				{
				case basic_value_type::number_bool:
				case basic_value_type::number_float:
				case basic_value_type::list:
				case basic_value_type::tuple:
					oss << "first column value type should be int or string" << endl;
					return oss.str();
				default:
					break;
				}
			}
			if (get_header_idx(cur_header_name) != 0)
			{
				oss << "duplicated header name " << cur_header_name << endl;
				return oss.str();
			}
			header_column_index[cur_header_name] = column_idx;
			column_idx += 1;
		}
		return {};
	}
	std::uint32_t typed_worksheet::value_begin_row() const
	{
		// 第一行 名字
		// 第二行 格式
		// 第三行 注释
		return 4;
	}
	std::string typed_worksheet::convert_cells_to_json()
	{
		m_cell_json_values.clear();
		auto value_begin_row_idx = value_begin_row();
		const auto& all_row_info = get_all_row();
		// 默认第0个是无效数据
		m_cell_json_values.emplace_back();
		if (value_begin_row_idx > max_rows)
		{
			return {};
		}
		m_cell_json_values.reserve(2 * (max_rows + 1 - value_begin_row_idx));
		
		m_cell_value_indexes = std::vector<std::uint32_t>((max_rows + 1 - value_begin_row_idx) * m_typed_headers.size(), 0);
		std::unordered_map<std::uint32_t, std::uint32_t> temp_map_from_ss_idx_to_json_idx;
		temp_map_from_ss_idx_to_json_idx[std::numeric_limits<std::uint32_t>::max()] = 0;
		std::ostringstream oss;
		for (std::uint32_t i = value_begin_row(); i < all_row_info.size(); i++)
		{
			for (std::uint32_t j = 1; j < all_row_info[i].size(); j++)
			{
				auto cur_cell_value_idx = get_cell_value_index_pos(i, j);
				auto cur_cell_ss_idx = get_cell_shared_string_idx(i, j);
				if (cur_cell_ss_idx == std::numeric_limits<std::uint32_t>::max())
				{
					continue;
				}
				auto cur_json_map_iter = temp_map_from_ss_idx_to_json_idx.find(cur_cell_ss_idx);
				if (cur_json_map_iter != temp_map_from_ss_idx_to_json_idx.end())
				{
					m_cell_value_indexes[cur_cell_value_idx] = cur_json_map_iter->second;
					continue;
				}
				string_view cur_cell_str = get_cell(i, j);
				if (cur_cell_str.empty())
				{
					continue;
				}
				std::string temp_extend_str;
				if (!json::accept(cur_cell_str))
				{
					oss << "\"" << cur_cell_str << "\"";
					temp_extend_str = oss.str();
					cur_cell_str = temp_extend_str;
					oss.clear();
					oss.str("");
				}
				if (!json::accept(cur_cell_str))
				{
					oss << "cant convert cell (" << i << ", " << j << ") with value " << cur_cell_str << " to json" << std::endl;
					return oss.str();
				}
				auto cur_cell_json = json::parse(cur_cell_str);
				if (!m_typed_headers[j].type_desc->validate(cur_cell_json))
				{
					bool check_fail = true;
					if (m_typed_headers[j].type_desc->m_type == basic_value_type::number_bool)
					{
						// excel 会把true false 自动转换为1 0
						if (cur_cell_json.is_number_unsigned())
						{
							auto cur_cell_int_json = cur_cell_json.get<std::uint32_t>();
							if (cur_cell_int_json == 0)
							{
								cur_cell_json = false;
								check_fail = false;
							}
							else if (cur_cell_int_json == 1)
							{
								cur_cell_json = true;
								check_fail = false;
							}
						}
					}
					if (check_fail)
					{
						oss << "cant validate cell (" << i << "," << j << ") with value " << cur_cell_str << " for header type " << m_typed_headers[i].type_desc->encode() << std::endl;
						return oss.str();
					}
					
				}
				m_cell_json_values.push_back(cur_cell_json);
				m_cell_value_indexes[cur_cell_value_idx] = m_cell_json_values.size() - 1;
				temp_map_from_ss_idx_to_json_idx[cur_cell_ss_idx] = m_cell_json_values.size() - 1;
			}
			
		}
		return {};
	}
	typed_worksheet::typed_worksheet(const vector<cell>& all_cells, uint32_t in_sheet_id, string_view in_sheet_name, const workbook<typed_worksheet>* in_workbook)
	: worksheet(all_cells, in_sheet_id, in_sheet_name, reinterpret_cast<const workbook<worksheet>*>(in_workbook))
	{

	}
	const workbook<typed_worksheet>* typed_worksheet::get_workbook() const
	{
		return reinterpret_cast<const workbook<typed_worksheet>*>(m_workbook);
	}
	std::string typed_worksheet::after_load_process()
	{
		auto temp_err = convert_typed_header();
		if(!temp_err.empty() )
		{
			return temp_err;
		}
		return convert_cells_to_json();
		
		
	}
	typed_worksheet::~typed_worksheet()
	{
		
	}
	const vector<typed_header>& typed_worksheet::get_typed_headers() const
	{
		return m_typed_headers;
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

	const json& typed_worksheet::get_typed_cell_value(uint32_t row_idx, uint32_t column_idx) const
	{
		if (row_idx == 0 || column_idx == 0)
		{
			return m_cell_json_values[0];
		}
		if (row_idx < value_begin_row() || row_idx > max_rows)
		{
			return m_cell_json_values[0];
		}
		if (column_idx > max_columns)
		{
			return m_cell_json_values[0];
		}
		return m_cell_json_values[m_cell_value_indexes[get_cell_value_index_pos(row_idx, column_idx)]];
	}
	
	
	bool typed_worksheet::check_header_match(const unordered_map<string_view, const typed_header*>& other_headers, string_view index_column_name) const
	{
		if (m_typed_headers.size() < 2)
		{
			std::cout << "current sheet doesnt has headers " << std::endl;
			return false;
		}
		if(m_typed_headers[1].header_name != index_column_name)
		{
			cout << "index column name mismatch input " << index_column_name << " current " << m_typed_headers[1].header_name << endl;
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
			if (header_idx >= m_typed_headers.size())
			{
				cout << " cant find header " << i.second->header_name << endl;
				return false;
			}
			if (!m_typed_headers[header_idx].type_desc)
			{
				cout << " cant find header " << i.second->header_name << endl;
				return false;
			}
			if(!(m_typed_headers[header_idx] == *i.second))
			{
				cout << "header type mismatch for  " << i.second->header_name << endl;
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
	
	std::vector<std::uint32_t> typed_worksheet::get_header_index_vector(const std::vector<std::string_view>& header_names) const
	{
		std::vector<std::uint32_t> result;
		for (const auto& i : header_names)
		{
			result.push_back(get_header_idx(i));
		}
		return result;
	}
	std::uint32_t typed_worksheet::get_cell_value_index_pos(std::uint32_t row_idx, std::uint32_t column_idx) const
	{
		return (row_idx - value_begin_row()) * max_columns + column_idx;
	}
}