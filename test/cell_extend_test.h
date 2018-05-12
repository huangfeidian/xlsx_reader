#include <xlsx_cell_extend.h>
#include <iostream>
using namespace std;
using namespace xlsx_reader;
void test_type_parse()
{
	cout << *extend_node_value_constructor::parse_type("list(ref(color), 3)") << endl;
    vector<string> valid_inputs = {
        "int",
        "str",
        "date",
        "time",
        "datetime",
        "bool",
        "uint32",
        "int",
        "uint64",
        "int64",
        "float",
        "double",
        "ref(colors)",
        "ref(rgb, colors)",
        "list(int, 2)",
        "list(str, 2, #)",
        "list(str, 0, #)"
        "tuple(int, str)",
        "tuple(str, str, #)",
        "list(ref(color), 3)",
        "list(list(int, 3), 3)",
        "list(list(int, 0), 3)",
        "list(list(int, 0), 0, #)",
        "tuple(tuple(int, int), tuple(str, str))",
        "tuple(tuple(int, int), tuple(str, str), #)",
        "tuple(list(int, 3), list(str, 3))",
        "tuple(ref(color), ref(color), ref(color))",
        "list(tuple(ref(color), ref(color, all_rgb), #), 3, ?)",
    };
    for(const auto& i : valid_inputs)
    {
        auto current_type = extend_node_value_constructor::parse_type(string_view(i));
		if (current_type)
		{
			cout << *current_type << endl;
		}
		else
		{
			cout << "fail for " << i << endl;
		}
        
    }
}