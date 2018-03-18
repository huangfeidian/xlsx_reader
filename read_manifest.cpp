#include "xlsx_relation.h"
#include "miniz.h"
#include "zip_utils.h"
#include <iostream>
#include <filesystem>
#include "xlsx_utils.h"

using namespace std;
using namespace std::experimental::filesystem;


int main(int argc, char** argv)
{
	/*const string file_name(argv[1]);*/
	const string file_name("wode.xlsx");
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
	auto share_string_content_iter = file_contents.find(shared_string_path);
	vector<share_string_type> all_share_strings;
	if (share_string_content_iter != file_contents.cend())
	{
		all_share_strings = read_shared_strings(share_string_content_iter->second);
	}

}