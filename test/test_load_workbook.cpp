#include <xlsx_workbook.h>
#include <iostream>
#include <fstream>
using namespace std;
using namespace xlsx_reader;

int main(int argc, char** argv)
{
    string file_name(argv[1]);
    auto archive_content = make_shared<archive>(file_name);
    workbook current_workbook(archive_content);
	string output_filename = "workbook.txt";
	ofstream output_stream(output_filename);
	output_stream <<current_workbook<<endl;
}