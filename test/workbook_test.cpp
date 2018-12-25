#include <xlsx_workbook.h>
#include <xlsx_typed_worksheet.h>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;
using namespace xlsx_reader;

struct Color
{
	int r;
	int g;
	int b;
	Color(int in_r, int in_g, int in_b)
		:r(in_r), g(in_g), b(in_b)
	{

	}
};
std::optional<Color> read_color_from_cell(const typed_worksheet& cur_worksheet, const typed_cell& cell_value)
{
	static unordered_map<string_view, unordered_map<string_view, Color>> color_caches;
	if (!cell_value.cur_typed_value)
	{
		return std::nullopt;
	}
	switch (cell_value.cur_typed_value->type_desc->_type)
	{
	case basic_node_type_descriptor::list:
	case basic_node_type_descriptor::tuple:
	{
		auto opt_color = cell_value.cur_typed_value->expect_value<std::tuple<int, int, int>>();
		if (!opt_color)
		{
			return std::nullopt;
		}
		auto real_color = opt_color.value();
		return Color(get<0>(real_color), get<1>(real_color), get<2>(real_color));
	}

	case basic_node_type_descriptor::ref_id:
	{
		auto cur_type_detail = cell_value.cur_typed_value->type_desc->get_ref_detail_t();
		if (!cur_type_detail)
		{
			return std::nullopt;
		}
		auto ref_detail = cur_type_detail.value();
		auto cur_workbook = cur_worksheet.get_workbook();
		auto[cur_wb_name, cur_ws_name, cur_ref_type] = ref_detail;
		auto ref_value = cell_value.cur_typed_value;
		auto ref_row_value = cur_worksheet.get_ref_row(cur_ws_name, ref_value);
		if (!ref_row_value)
		{
			return std::nullopt;
		}
		const auto& row_value = ref_row_value.value().get();
		
		if (row_value.size() == 2)
		{
			auto other_sheet_idx = cur_workbook->get_sheet_index_by_name(cur_ws_name);
			if (!other_sheet_idx)
			{
				return std::nullopt;
			}
			const auto& other_sheet = cur_workbook->get_worksheet(other_sheet_idx.value());
			return read_color_from_cell(other_sheet, *(row_value.rbegin()->second));
		}
		else
		{
			if (row_value.size() != 4)
			{
				return std::nullopt;
			}
			else
			{
				vector<int32_t> rgb_values;
				for (int i = 1; i < 4; i++)
				{
					auto cur_cell_value_iter = row_value.find(i);
					if (cur_cell_value_iter == row_value.end())
					{
						return std::nullopt;
					}
					auto cur_cell_value = cur_cell_value_iter->second;
					auto cur_int_opt = cur_cell_value->cur_typed_value->expect_value<std::int32_t>();
					if (!cur_int_opt)
					{
						return std::nullopt;
					}
					rgb_values.push_back(cur_int_opt.value());
				}
				return Color(rgb_values[0], rgb_values[1], rgb_values[2]);
			}
		}

	}
	default:
		return std::nullopt;

	}
	return std::nullopt;
}

//int main(int argc, char** argv)
int main(void)
{
	//string file_name(argv[1]);
	string file_name = "../examples/xlsx_shape_test2.xlsx";
	auto archive_content = make_shared<archive>(file_name);
	workbook<typed_worksheet> current_workbook(archive_content);

	// ref test
	//auto ref_test_sheet_idx = current_workbook.get_sheet_index_by_name("ref_test");
	//if(!ref_test_sheet_idx)
	//{
	//	cerr<<"cant find sheet ref_test"<<endl;
	//	return 0;
	//}
	//auto& the_sheet = current_workbook.get_worksheet(ref_test_sheet_idx.value());
	//for(int i = 0; i<the_sheet.get_all_typed_row_info().size(); i++)
	//{
	//	auto cur_ref_key= the_sheet.get_typed_cell(i, 1);
	//	auto the_ref_value = the_sheet.get_ref_row("all_colors", cur_ref_key->cur_typed_value);
	//	if(!the_ref_value)
	//	{
	//		cerr<<"cant find ref value for key "<< *cur_ref_key->cur_typed_value<<endl;
	//	}
	//}
	//return 1;
	auto other_sheet_idx = current_workbook.get_sheet_index_by_name("tile_1");
	auto current_worksheet = current_workbook.get_worksheet(other_sheet_idx.value());
	auto cur_color_cell = current_worksheet.get_typed_cell(0, 5);
	auto result_color = read_color_from_cell(current_worksheet, *cur_color_cell);


	return 0;

}