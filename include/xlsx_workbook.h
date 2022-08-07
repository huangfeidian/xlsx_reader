#pragma once
#include "xlsx_worksheet.h"
#include <memory>
#include "xlsx_archive.h"
#include <optional>
#include <iostream>

namespace spiritsaway::xlsx_reader
{
	template <typename worksheet_t> 
	class workbook
	{
	public:
		std::vector<std::unique_ptr<worksheet_t>> m_worksheets;
		std::vector<sheet_desc> sheet_relations; 
		
	private:
		std::vector<std::string_view> shared_string;
		spiritsaway::memory::arena string_arena;
		std::unordered_map<std::string_view, std::uint32_t> shared_string_indexes;
		std::unordered_map<std::string_view, std::uint32_t> sheets_name_map;
		std::uint32_t get_index_for_string(std::string_view in_str)
		{
			auto iter = shared_string_indexes.find(in_str);
			if (iter != shared_string_indexes.end())
			{
				return iter->second;
			}
			else
			{
				auto cur_view_sz = in_str.size();
				std::string_view cur_str_view;
				if (cur_view_sz)
				{
					char* new_buffer = string_arena.get<char>(cur_view_sz);
					std::copy(in_str.cbegin(), in_str.cend(), new_buffer);
					cur_str_view = std::string_view(new_buffer, cur_view_sz);
				}
				
				shared_string.push_back(cur_str_view);
				shared_string_indexes[cur_str_view] = shared_string.size() - 1;
				return shared_string.size() - 1;
			}
		}
		
	public:
		std::optional<std::uint32_t> get_sheet_index_by_name(std::string_view sheet_name) const
		{
			auto iter = sheets_name_map.find(sheet_name);
			if(iter == sheets_name_map.end())
			{
				return std::nullopt;
			}
			else
			{
				return iter->second;
			}
		}
		const worksheet_t& get_worksheet(std::uint32_t sheet_idx) const
		{
			return *(m_worksheets[sheet_idx]);
		}
		std::uint32_t get_worksheet_num() const
		{
			return m_worksheets.size();
		}
		std::string_view workbook_name;
		std::string_view get_workbook_name() const
		{
			return workbook_name;
		}
		std::string_view get_shared_string(std::uint32_t ss_idx) const
		{
			if (ss_idx >= shared_string.size())
			{
				return std::string_view();
			}
			else
			{
				return std::string_view(shared_string[ss_idx]);
			}
		}

		workbook(std::shared_ptr<archive> in_archive)
			:string_arena(4 * 1024)
		{
			archive_content = in_archive;
	
			in_archive->get_shared_string_view(string_arena, shared_string);
			shared_string.insert(shared_string.begin(), std::string_view());
			for (std::uint32_t i = 0; i < shared_string.size(); i++)
			{
				shared_string_indexes[shared_string[i]] = i;
			}
			sheet_relations = in_archive->get_all_sheet_relation();
			// we should load all shared strings here incase any relocation of the shared_string vector
			// invalidate the string_views that ref the shared string
			for (std::uint32_t i = 0; i < sheet_relations.size(); i++)
			{
				get_cells_for_sheet(i + 1);
			}
			shared_string.shrink_to_fit();

			// from now the shared_string begins to function
			for(std::uint32_t i = 0; i < sheet_relations.size(); i++)
			{
				auto cur_worksheet = new worksheet_t(get_cells_for_sheet(i + 1), std::get<1>(sheet_relations[i]), std::get<0>(sheet_relations[i]), this);
				auto cur_sheet_load_err = cur_worksheet->after_load_process();
				if (!cur_sheet_load_err.empty())
				{
					std::cerr << "load sheet " << cur_worksheet->get_name() << "fail with error " << cur_sheet_load_err << std::endl;
					delete cur_worksheet;
					continue;
				}
				m_worksheets.emplace_back(cur_worksheet);
				auto current_sheet_name = cur_worksheet->get_name();
				sheets_name_map[current_sheet_name] = i;
			}
			after_load_process();
		}
		
		friend std::ostream& operator<<(std::ostream& output_stream, const workbook& in_workbook)
		{
			output_stream<<"workbook name:"<<string(in_workbook.get_workbook_name())<<std::endl;
			for(const auto& one_worksheet: in_workbook.m_worksheets)
			{
				output_stream<<*one_worksheet<<std::endl;
			}
			return output_stream;
		}
	protected:
		std::shared_ptr<archive> archive_content;
		std::shared_ptr<tinyxml2::XMLDocument> get_sheet_xml(std::uint32_t sheet_idx) const
		{
			auto sheet_path = "xl/worksheets/sheet" + std::to_string(sheet_idx) + ".xml";
			return archive_content->get_xml_document(sheet_path);
		}
		
		void after_load_process()
		{
			// std::cout<<"Workbook "<<workbook_name<<" total sheets "<<m_worksheets.size()<<std::endl;
		}
		std::unordered_map<std::uint32_t, std::vector<cell>> all_cells;

		const std::vector<cell>& get_cells_for_sheet(std::uint32_t sheet_idx)
		{
			auto cache_iter = all_cells.find(sheet_idx);
			if(cache_iter != all_cells.end())
			{
				return cache_iter->second;
			}
			auto& result = all_cells[sheet_idx];
			auto sheet_doc = get_sheet_xml(sheet_idx);

			auto worksheet_node = sheet_doc->FirstChildElement("worksheet");
			auto sheet_data_node = worksheet_node->FirstChildElement("sheetData");
			auto row_node = sheet_data_node->FirstChildElement("row");
			while(row_node)
			{
				uint32_t row_index = std::stoi(row_node->Attribute("r"));
				auto cell_node = row_node->FirstChildElement("c");
				while(cell_node)
				{
					uint32_t col_idx = row_column_tuple_from_string(cell_node->Attribute("r")).second;
					auto v_node = cell_node->FirstChildElement("v");
					if (!v_node)
					{
						break;
					}

					std::string_view current_value = cell_node->FirstChildElement("v")->GetText();
					current_value = strip_blank(current_value);
					auto type_attr = cell_node->Attribute("t");
					std::uint32_t ss_idx = 0;
					if(type_attr && *type_attr == 's')
					{
						// 由于我们在前面放了一个空字符串 所以后续的字符串索引都要加1
						ss_idx = std::stoi(std::string(current_value)) + 1;
					}
					else
					{
						ss_idx = get_index_for_string(current_value);
					}
					cell_node = cell_node->NextSiblingElement("c");
					result.emplace_back(row_index, col_idx, ss_idx);

				}
				row_node = row_node->NextSiblingElement("row");
			}
			return result;
		}
	};
}
