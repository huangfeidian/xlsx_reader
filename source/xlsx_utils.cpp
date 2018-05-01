#include <xlsx_utils.h>
#include <tinyxml2.h>
#include <iostream>
#include <tuple>
#include <miniz.h>
#include <filesystem>
#include <fstream>
#include <streambuf>
#include <vector>
#include <sstream>
#include <xlsx_cell.h>

namespace xlsx_reader
{

using namespace std;
using namespace std::experimental::filesystem;
using namespace tinyxml2;

uint32_t column_index_from_string(const string& column_string)
{

	if (column_string.length() > 3 || column_string.empty())
    {
        return 0;
    }

    uint32_t column_index = 0;
    int place = 1;

    for (int i = static_cast<int>(column_string.length()) - 1; i >= 0; i--)
    {
        if (!std::isalpha(column_string[static_cast<std::size_t>(i)]))
        {
            return 0;
        }

        auto char_index = std::toupper(column_string[static_cast<std::size_t>(i)]) - 'A';

        column_index += static_cast<uint32_t>((char_index + 1) * place);
        place *= 26;
    }

    return column_index;
}
pair<uint32_t, uint32_t> row_column_tuple_from_string(const string& tuple_string)
{
	//sample input A1
	uint32_t column_char_size = 0;
	while (isalpha(tuple_string[column_char_size]))
	{
		column_char_size++;
	}
	uint32_t column_value = column_index_from_string(tuple_string.substr(0, column_char_size));
	uint32_t row_value = stoi(tuple_string.substr(column_char_size));
	return make_pair(row_value, column_value);
}
std::string column_string_from_index(uint32_t column_index)
{
    // these indicies corrospond to A->ZZZ and include all allowed
    // columns
    uint32_t min_col = 1;
	uint32_t max_col = 26 + 26*26 + 26*26*26 + 1;
	if(column_index < min_col || column_index > max_col)
	{
		return "ZZZZ";
	}
    int temp = static_cast<int>(column_index);
    std::string column_letter = "";

    while (temp > 0)
    {
        int quotient = temp / 26, remainder = temp % 26;

        // check for exact division and borrow if needed
        if (remainder == 0)
        {
            quotient -= 1;
            remainder = 26;
        }

        column_letter = std::string(1, char(remainder + 64)) + column_letter;
        temp = quotient;
    }

    return column_letter;
}

std::unordered_map<std::string, std::string> read_zip_to_memory(const std::string& file_name)
{
	unordered_map<string, string> result;
	path file_path(file_name);
	std::ifstream input_file_stream(file_path, std::ios::binary);
	input_file_stream.seekg(0, std::ios::end);
	string cur_all_content;
	cur_all_content.reserve(input_file_stream.tellg());
	input_file_stream.seekg(0, std::ios::beg);
	cur_all_content.assign(std::istreambuf_iterator<char>(input_file_stream), std::istreambuf_iterator<char>());
	std::cout << "minic version " << MZ_VERSION << std::endl;
	std::cout << "string size " << cur_all_content.size() << "file path " << current_path() << std::endl;
	mz_zip_archive cur_archive;

	memset(&cur_archive, 0, sizeof(cur_archive));

	int status = mz_zip_reader_init_mem(&cur_archive, static_cast<const void*>(cur_all_content.c_str()), cur_all_content.size(), 0);
	if (!status)
	{
		std::cerr << "invalid zip file " << file_path << " status " << cur_archive.m_last_error << std::endl;
		return result;
	}

	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&cur_archive); i++)
	{
		mz_zip_archive_file_stat cur_file_stat;
		if (!mz_zip_reader_file_stat(&cur_archive, i, &cur_file_stat))
		{
			std::cerr << "mz_zip_reader_file_stat failed" << std::endl;
			return result;
		}
		std::cout << " Filename " << cur_file_stat.m_filename << "comment " << cur_file_stat.m_comment <<
			" uncompressed size " << cur_file_stat.m_uncomp_size << " compressed size " << cur_file_stat.m_comp_size
			<< " is dir " << mz_zip_reader_is_file_a_directory(&cur_archive, i) << std::endl;

		size_t cur_uncom_size = 0;
		void* p = mz_zip_reader_extract_file_to_heap(&cur_archive, cur_file_stat.m_filename, &cur_uncom_size, 0);
		if (!p)
		{
			std::cerr << "memory allocation fail for file " << cur_file_stat.m_filename << std::endl;
			return result;
		}
		std::cout << "success extracted file " << cur_file_stat.m_filename << std::endl;
		char* new_p = (char*)p;
		string output_buffer(new_p, new_p + cur_file_stat.m_uncomp_size);

		result[string(cur_file_stat.m_filename)] = output_buffer;
		free(p);
	}
	mz_zip_reader_end(&cur_archive);
	return result;

}

vector<relation_desc> read_relations_from_content(string_view current_content)
{
	vector<relation_desc> all_relations;
	XMLDocument relation_doc;
	relation_doc.Parse(current_content.data(), current_content.size());
	auto relation_total_node = relation_doc.FirstChildElement("Relationships");
	auto brother_begin = relation_total_node->FirstChildElement("Relationship");
	while (brother_begin)
	{
		string current_id(brother_begin->Attribute("Id"));
		string current_type(brother_begin->Attribute("Type"));
		string current_target(brother_begin->Attribute("Target"));
		brother_begin = brother_begin->NextSiblingElement("Relationship");

		all_relations.push_back(make_tuple(current_id, current_type, current_target));
	}
	return all_relations;
}

vector<sheet_desc> read_sheets_from_workbook(string_view current_content)
{
	vector<sheet_desc> all_sheets;
	XMLDocument workbook;
	workbook.Parse(current_content.data(), current_content.size());
	auto workbook_node = workbook.FirstChildElement("workbook");
	auto sheets_node = workbook_node->FirstChildElement("sheets");
	auto sheets_begin = sheets_node->FirstChildElement("sheet");
	while (sheets_begin)
	{
		string current_sheet_name(sheets_begin->Attribute("name"));
		string current_sheet_id(sheets_begin->Attribute("sheetId"));
		string current_relation_id(sheets_begin->Attribute("r:id"));
		all_sheets.emplace_back(make_tuple(current_sheet_name, stoi(current_sheet_id), current_relation_id));
		//auto attr_begin = sheets_begin->FirstAttribute();
		//while (attr_begin)
		//{
		//	cout << "attr name " << attr_begin->Name() << " value is " << attr_begin->Value() << endl;
		//	attr_begin = attr_begin->Next();
		//}
		sheets_begin = sheets_begin->NextSiblingElement("sheet");
	}
	return all_sheets;
}
vector<string_view> read_shared_string(string_view current_content)
{
	vector<string_view> all_share_strings;
	XMLDocument string_doc;
	string_doc.Parse(current_content.data(), current_content.size());
	auto share_total_node = string_doc.FirstChildElement("sst");
	auto share_string_begin = share_total_node->FirstChildElement("si");
	while (share_string_begin)
	{
		auto current_value = share_string_begin->FirstChildElement("t")->GetText();
		string current_type = share_string_begin->FirstChildElement("phoneticPr")->Attribute("type");
		// cout << "value " << current_value << " has type " << current_type << endl;
		all_share_strings.emplace_back(current_value);
		share_string_begin = share_string_begin->NextSiblingElement("si");
	}
	return all_share_strings;
}
vector<vector<cell*>> load_cells_from_string(string_view input_string, const vector<string_view>& shared_string_table)
{
	XMLDocument sheet_doc;
	vector<vector<cell*>> result;
	sheet_doc.Parse(input_string.data(), input_string.size());
	auto worksheet_node = sheet_doc.FirstChildElement("worksheet");
	auto sheet_data_node = worksheet_node->FirstChildElement("sheetData");
	auto row_node = sheet_data_node->FirstChildElement("row");
	while(row_node)
	{
		uint32_t row_index = stoi(row_node->Attribute("r"));
		vector<cell*> current_row_cells;
		auto cell_node = row_node->FirstChildElement("c");
		while(cell_node)
		{
			uint32_t col_idx = row_column_tuple_from_string(cell_node->Attribute("r")).second;
			auto cur_cell = new cell(row_index, col_idx);
			auto current_value = cell_node->FirstChildElement("v")->GetText();
			auto type_attr = cell_node->Attribute("t");
			if(type_attr)
			{
				auto type_attr_v = string(type_attr);
				if(type_attr_v == "s")
				{
					// shared str
					cur_cell->set_value(shared_string_table[stoi(current_value)]);
				}
				else if(type_attr_v == "str")
				{
					// simple_str
					cur_cell->set_value(current_value);
				}
				else if(type_attr_v == "b")
				{
					// BOOL
					auto bool_value = stoi(current_value) == 1;
					cur_cell->set_value(bool_value);
				}
				else if(type_attr_v == "n")
				{
					//numeric
					auto double_value = stod(current_value);
					cur_cell->set_value(double_value);
				}
				else if(type_attr_v == "inlineStr")
				{
					// simple_str
					cur_cell->set_value(current_value);
				}
				else if(type_attr_v == "e")
				{
					// error
					cur_cell->from_error(current_value);
				}
				else
				{
					// simple_str
					cur_cell->set_value(current_value);
				}
			}
			else
			{
				//numeric
				auto double_value = stod(current_value);
				cur_cell->set_value(double_value);
			}

			cell_node = cell_node->NextSiblingElement("c");
			current_row_cells.push_back(cur_cell);

		}
		result.push_back(current_row_cells);
		row_node = row_node->NextSiblingElement("row");
	}
	return result;
}
}