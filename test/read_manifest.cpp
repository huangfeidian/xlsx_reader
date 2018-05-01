#include <miniz.h>
#include <iostream>
#include <filesystem>
#include <xlsx_utils.h>

using namespace std;
using namespace std::experimental::filesystem;
using namespace xlsx_reader;

int read_manifest(string file_name)
{
	auto file_contents = read_zip_to_memory(file_name);
	for (const auto i : file_contents)
	{
		cout << "has file " << i.first << " with len " << i.second.size() << endl;
	}

	auto relation_path = "_rels/.rels";
	auto current_content = file_contents[relation_path];
	
	auto all_relations = read_relations_from_content(current_content);
	string work_book_path = "";
	for (const auto& i : all_relations)
	{
		if (std::get<1>(i) == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument")
		{
			work_book_path = std::get<2>(i);
			break;
		}
	}
	if (work_book_path.size() == 0)
	{
		return 0;
	}
	auto workbook_content = file_contents[work_book_path];

	auto all_sheets = read_sheets_from_workbook(workbook_content);
	
	auto sheet_relations = read_relations_from_content(file_contents["xl/_rels/workbook.xml.rels"]);
	string shared_string_path = "xl/sharedStrings.xml";

}