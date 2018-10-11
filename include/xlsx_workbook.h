#pragma once
#include <xlsx_worksheet.h>
#include <memory>
#include <xlsx_archive.h>
#include <optional>
namespace xlsx_reader
{
	template <typename worksheet_t> 
	class workbook
	{
	public:
		std::vector<std::unique_ptr<worksheet_t>> _worksheets;
		std::vector<sheet_desc> sheet_relations; 
		std::vector<std::string_view> shared_string;
	private:
		std::unordered_map<std::string_view, std::uint32_t> sheets_name_map;
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
			return *(_worksheets[sheet_idx]);
		}
		std::uint32_t get_worksheet_num() const
		{
			return _worksheets.size();
		}
		std::string_view workbook_name;
		std::string_view get_workbook_name() const
		{
			return workbook_name;
		}
		workbook(std::shared_ptr<archive> in_archive)
		{
			archive_content = in_archive;
		
			shared_string = in_archive->get_shared_string();

			sheet_relations = in_archive->get_all_sheet_relation();

			for(int i = 0; i < sheet_relations.size(); i++)
			{
				auto cur_worksheet = new worksheet_t(get_cells_for_sheet(i + 1), get<1>(sheet_relations[i]), get<0>(sheet_relations[i]), this);
				cur_worksheet->after_load_process();
				_worksheets.emplace_back(cur_worksheet);
				auto current_sheet_name = cur_worksheet->get_name();
				sheets_name_map[current_sheet_name] = i;
			}
			after_load_process();
		}
		friend void to_json(json& j, const workbook& in_workbook)
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
		friend std::ostream& operator<<(std::ostream& output_stream, const workbook& in_workbook)
		{
			output_stream<<"workbook name:"<<string(in_workbook.get_workbook_name())<<endl;
			for(const auto& one_worksheet: in_workbook._worksheets)
			{
				output_stream<<*one_worksheet<<endl;
			}
			return output_stream;
		}
	protected:
		std::shared_ptr<archive> archive_content;
		const tinyxml2::XMLDocument* get_sheet_xml(std::uint32_t sheet_idx) const
		{
			auto sheet_path = "xl/worksheets/sheet" + to_string(sheet_idx) + ".xml";
			return archive_content->get_xml_document(sheet_path);
		}
		
		void after_load_process()
		{
			cout<<"Workbook "<<workbook_name<<" total sheets "<<_worksheets.size()<<endl;
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
							cur_cell.set_value(shared_string[stoi(current_value)]);
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
	};
}