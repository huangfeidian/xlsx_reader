#include "xlsx_utils.h"
#include "tinyxml2.h"
#include <iostream>
#include <tuple>
using namespace tinyxml2;
using namespace std;
vector<relation_desc> read_relations_from_content(const string& current_content)
{
	vector<relation_desc> all_relations;
	XMLDocument relation_doc;
	relation_doc.Parse(current_content.c_str());
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

vector<sheet_desc> read_sheets_from_workbook(const string& current_content)
{
	vector<sheet_desc> all_sheets;
	XMLDocument workbook;
	workbook.Parse(current_content.c_str());
	auto workbook_node = workbook.FirstChildElement("workbook");
	auto sheets_node = workbook_node->FirstChildElement("sheets");
	auto sheets_begin = sheets_node->FirstChildElement("sheet");
	while (sheets_begin)
	{
		string current_sheet_name(sheets_begin->Attribute("name"));
		string current_sheet_id(sheets_begin->Attribute("sheetId"));
		string current_relation_id(sheets_begin->Attribute("r:id"));
		all_sheets.emplace_back(make_tuple(current_sheet_name, current_sheet_id, current_relation_id));
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
vector<cell_final_value> read_shared_strings(const string& current_content)
{
	vector<cell_final_value> all_share_strings;
	XMLDocument string_doc;
	string_doc.Parse(current_content.c_str());
	auto share_total_node = string_doc.FirstChildElement("sst");
	auto share_string_begin = share_total_node->FirstChildElement("si");
	while (share_string_begin)
	{
		string current_value = share_string_begin->FirstChildElement("t")->GetText();
		string current_type = share_string_begin->FirstChildElement("phoneticPr")->Attribute("type");
		// cout << "value " << current_value << " has type " << current_type << endl;
		all_share_strings.emplace_back(current_value, current_type);
		share_string_begin = share_string_begin->NextSiblingElement("si");
	}
	return all_share_strings;
}
std::vector<std::vector<cell_raw_value>> read_worksheet(const std::string& current_content)
{

}