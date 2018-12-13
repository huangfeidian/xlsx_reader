
#include <xlsx_json.h>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
using namespace xlsx_reader;
bool json_test_type_parse()
{
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
        "ref(colors, str)",
        "ref(rgb, colors, str)",
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
        "tuple(ref(color, str), ref(color, str), ref(color, str))",
        "list(tuple(ref(color, str), ref(color, all_rgb, str), #), 3, ?)",
    };
	bool failed = false;
    for(const auto& i : valid_inputs)
    {
        auto current_type = typed_node_value_constructor::parse_type(string_view(i));
		if (current_type)
		{
			cout << *current_type << endl;
            cout << "json type is "<< json(*current_type)<<endl;
		}
		else
		{
			cout << "fail for " << i << endl;
			failed = true;
		}
        
    }
	return failed;
}

bool json_test_type_value_parse()
{
	unordered_map<string, vector<string>> typed_values = {
		{"int", {"0", "1", "-1", "-1314598", "314159"}},
		{"uint32", {"0", "1", "10", "314159"}},
		{"uint64", {"0", "1", "200000000000"}},
		{"int64", {"0", "-1", "1", "20000000000000", "-20000000000000"}},
		{"float", {"0.0", "-0.0", "1.0", "-1.0", "3.14159", "-3141519"}},
		{"double", {"0.0", "-0.0", "3.141592657385", "-3.141592657384"}},
		{"ref(colors, str)", {"rgb1", "rgb2", "rgb3"}},
		{ "ref(colors, int)",{ "1", "2", "3" } },
		{ "ref(colors, color_sheet, str)",{ "rgb1", "rgb2", "rgb3" } },
		{"tuple(int, str)", {"(3, 5)", "(4, hehe)", "(5, -1)", "(0, fail)"}},
		{"tuple(str, str, #)", {"(wala , #hehe)", "(wawa,#hehe)", "( wawa, # hehe)", "(	wawa	, #    hehe )"} },
		{"list(ref(color, str), 3)", {"(rgb1,rgb2, rgb3)", "(	rgb1 , rgb 2, rgb 3)"} },
		{"list(int, 3)", {"(1,2,3)"}},
		{"list(list(int, 3), 3)", {"((1,2,3),(2,3,4),(3,4,5))"}},
		{"list(list(int, 0), 3)", {"((1),(1,2),(1,2,3))"}},
		{"list(list(int, 0), 0, #)",{ "((1)#(1,2)#(1,2,3)#(1,2,3,4))" } },
		{"tuple(tuple(int,int), tuple(str,str))", {"((1,2),(h, k))"}},
		{"tuple(tuple(int,int), tuple(str, str), #)", {"((1,2)#(h,k))"}},
		{"tuple(list(int, 3), list(int, 3))", {"((1,2,3),(2,3,4))"}},
		{"tuple(ref(color, str), ref(color, str), ref(color, str))", {"(rgb1, rgb2, rgb3)"}},
		{"list(tuple(ref(color, str), ref(color, all_rgb, str), #),3,?)", {"((rgb1#rgb2)?(rgb1#rgb2)?(rgb1#rgb2))"}}
	};
	bool failed = false;
	for (const auto& i : typed_values)
	{
		auto type_str = i.first;
		auto value_strs = i.second;
		auto current_type = typed_node_value_constructor::parse_type(string_view(type_str));
		for (const auto & one_value : value_strs)
		{
			auto current_value = typed_node_value_constructor::parse_value_with_type(current_type, string_view(one_value));
			if (current_value)
			{
				cout << "parse type "<<*current_type<<" with value "<< one_value<<" match with result "<<*current_value << endl;
                cout<<"json value is "<< json(*current_value)<<endl;
			}
			else
			{
				cout << "fail for type " << *current_type << " with value " << one_value << endl;
				failed = true;
				return false;
			}
			
		}
		
	}
	return true;
}

int main(void)
{
	using namespace std;
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