# xlsx_reader 读取excel xlsx文件的工具

## 功能
可以读取xlsx文件的文本内容，到自定义的容器之中，这个容器中的数据可以导出为json。
还支持带schema的文本读取与验证，这个schema由https://github.com/huangfeidian/typed_string定义。

## 依赖
1. c++ 17
2. tinyxml2 https://github.com/leethomason/tinyxml2
3. miniz https://github.com/richgel999/miniz
4. json https://github.com/nlohmann/json
4. typed_string https://github.com/huangfeidian/typed_string

## 使用
参考test文件夹下的两个文件。整个工作流程分为了如下几步：
1. 通过xlsx文件路径加载zip文件， 因为xlsx文件其实就是zip文件
2. 将zip文件解析为workbook，主要是解析shared_string, 当前的解析比较简单，可能有不完备的地方，欢迎提bug
3. 解析完workbook之后，根据记录的每个sheet再去解析
4. 如果是带格式的worksheet，还提供了一些函数去转换格式化字符串
下面就是一个参考的代码
```c++
string file_name = "../examples/pi100000.xlsx";
int wait = 0;
cout << "size of typed value is " << sizeof(arena_typed_value) << endl;
auto archive_content = make_shared<archive>(file_name);
cout << "press any key to load the xlsx file" << endl;
cin >> wait;
workbook<typed_worksheet> current_workbook(archive_content);


cout << "press any key to show memory statistics" << endl;
cin >> wait;
for (const auto& one_sheet : current_workbook._worksheets)
{
    cout << "worksheet " << one_sheet->get_name() << " memory consumption is " << one_sheet->memory_consumption() << endl;
}
auto sheet_idx_opt = current_workbook.get_sheet_index_by_name("tile_1");
const typed_worksheet& cur_worksheet = current_workbook.get_worksheet(sheet_idx_opt.value());
vector<string_view> header_names = { "tile_id", "circle_id", "width", "sequence", "ref_color", "opacity", "filled" };
vector<uint32_t> header_indexes = cur_worksheet.get_header_index_vector(header_names);
for (std::uint32_t i = 0; i < header_names.size(); i++)
{
    std::cout << "head " << header_names[i] << " at " << header_indexes[i] << endl;
}
auto row_convert_result = cur_worksheet.try_convert_row<string_view, string_view, int, int, string_view, float, bool>(1, header_indexes);
auto [opt_tile_id, opt_circle_id, opt_width, opt_seq, opt_ref_color, opt_opacity, opt_filled] = row_convert_result;
```
