#include <xlsx_workbook.h>
#include <iostream>
using namespace std;
using namespace xlsx_reader;

int main(int argc, char** argv)
{
    //string file_name(argv[1]);
	string file_name("hehe.xlsx");
    workbook current_workbook(file_name);
    cout<<current_workbook<<endl;
}