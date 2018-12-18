#include <xlsx_workbook.h>
#include <xlsx_typed_worksheet.h>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
using namespace xlsx_reader;

//int main(int argc, char** argv)
int main(void)
{
	//string file_name(argv[1]);
	string file_name = "../examples/sample1.xlsx";
	auto archive_content = make_shared<archive>(file_name);
	workbook<typed_worksheet> current_workbook(archive_content);

	// ref test
	auto ref_test_sheet_idx = current_workbook.get_sheet_index_by_name("ref_test");
	if(!ref_test_sheet_idx)
	{
		cerr<<"cant find sheet ref_test"<<endl;
		return 0;
	}
	auto& the_sheet = current_workbook.get_worksheet(ref_test_sheet_idx.value());
	for(int i = 0; i<the_sheet.get_all_typed_row_info().size(); i++)
	{
		auto cur_ref_key= the_sheet.get_typed_cell(i, 1);
		auto the_ref_value = the_sheet.get_ref_row("all_colors", cur_ref_key->cur_typed_value);
		if(!the_ref_value)
		{
			cerr<<"cant find ref value for key "<< *cur_ref_key->cur_typed_value<<endl;
		}
	}
	return 1;

}