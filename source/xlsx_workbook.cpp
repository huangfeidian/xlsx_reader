#include <xlsx_workbook.h>
#include <tinyxml2.h>
#include <iostream>
namespace
{
	using namespace std;
	using namespace tinyxml2;
	using namespace xlsx_reader;
vector<sheet_desc> read_sheets_from_workbook(const XMLDocument* workbook_ptr)
{
	vector<sheet_desc> all_sheets;
	auto& workbook = *workbook_ptr;
	auto workbook_node = workbook.FirstChildElement("workbook");
	auto sheets_node = workbook_node->FirstChildElement("sheets");
	auto sheets_begin = sheets_node->FirstChildElement("sheet");
	while (sheets_begin)
	{
		string current_sheet_name(sheets_begin->Attribute("name"));
		string current_sheet_id(sheets_begin->Attribute("sheetId"));
		string current_relation_id(sheets_begin->Attribute("r:id"));
		all_sheets.emplace_back(make_tuple(current_sheet_name, stoi(current_sheet_id), current_relation_id));
		sheets_begin = sheets_begin->NextSiblingElement("sheet");
	}
	return all_sheets;
}
vector<string_view> read_shared_string(const XMLDocument* shared_table_xml_ptr)
{
	vector<string_view> all_share_strings;
	auto& string_doc = *shared_table_xml_ptr;
	auto share_total_node = string_doc.FirstChildElement("sst");
	auto share_string_begin = share_total_node->FirstChildElement("si");
	while (share_string_begin)
	{
		auto current_value = share_string_begin->FirstChildElement("t")->GetText();
		// cout << "value " << current_value << " has type " << current_type << endl;
		all_share_strings.emplace_back(current_value);
		share_string_begin = share_string_begin->NextSiblingElement("si");
	}
	return all_share_strings;
}
}

namespace xlsx_reader
{
	using namespace std;
	using namespace tinyxml2;
	const XMLDocument* workbook::get_sheet_xml(uint32_t sheet_idx) const
	{
		auto sheet_path = "xl/worksheets/sheet" + to_string(sheet_idx) + ".xml";
		return archive_content->get_xml_document(sheet_path);
	}
	uint32_t workbook::get_worksheet_num() const
	{
		return sheet_relations.size();
	}
	const worksheet& workbook::get_worksheet(uint32_t sheet_idx) const
	{
		return *(_worksheets[sheet_idx]);
	}
	string_view workbook::get_workbook_name() const
	{
		return workbook_name;
	}
	workbook::workbook(shared_ptr<archive> in_archive)
	{
		archive_content = in_archive;
		
		auto shared_string_table_path = "xl/sharedStrings.xml";
		shared_string = read_shared_string(archive_content->get_xml_document(shared_string_table_path));

		auto workbook_relation_path = "xl/workbook.xml";
		sheet_relations = read_sheets_from_workbook(archive_content->get_xml_document(workbook_relation_path));

		for(int i = 0; i < sheet_relations.size(); i++)
		{
			auto cur_worksheet = new worksheet(this, get<1>(sheet_relations[i]), get<0>(sheet_relations[i]));
			_worksheets.emplace_back(cur_worksheet);
		}
	}
	ostream& operator<<(ostream& output_stream, const workbook& in_workbook)
	{
		output_stream<<"workbook name:"<<string(in_workbook.get_workbook_name())<<endl;
		for(const auto& one_worksheet: in_workbook._worksheets)
		{
			output_stream<<*one_worksheet<<endl;
		}
		return output_stream;
	}

}