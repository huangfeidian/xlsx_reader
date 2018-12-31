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
	string file_name = "../examples/xlsx_shape_test.xlsx";
	int wait = 0;
	cout << "size of typed value is " << sizeof(typed_value) << endl;
	auto archive_content = make_shared<archive>(file_name);
	cin >> wait;
	workbook<typed_worksheet> current_workbook(archive_content);
	uint32_t workbook_memory = current_workbook.memory_details();
	
	cin >> wait;
	return 0;

}