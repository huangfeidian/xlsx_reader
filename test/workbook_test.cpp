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
	// test_type_parse();
	// test_type_value_parse();
	//json_test_type_parse();
	//json_test_type_value_parse();

}