#include <xlsx_workbook.h>
#include <xlsx_typed.h>
#include "cell_extend_test.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "json_test.h"
using namespace std;
using namespace xlsx_reader;

//int main(int argc, char** argv)
int main(void)
{
	 //string file_name(argv[1]);
	string file_name = "../examples/sample1.xlsx";
	auto archive_content = make_shared<archive>(file_name);
	workbook<typed_worksheet> current_workbook(archive_content);
	string output_filename = "workbook.txt";
	ofstream output_stream(output_filename);
	json j = current_workbook;
	 output_stream <<setw(4)<<j<<endl;
	// test_type_parse();
	// test_type_value_parse();
	//json_test_type_parse();
	//json_test_type_value_parse();

}