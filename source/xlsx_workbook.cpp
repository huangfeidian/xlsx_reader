#include <xlsx_workbook.h>
#include <xlsx_utils.h>
#include <tinyxml2.h>
#include <iostream>
namespace xlsx_reader
{
    using namespace std;
    using namespace tinyxml2;
    void workbook::load_workbook_from_string(const unordered_map<string, string>& unziped_archive)
    {
        auto shared_string_table_path = "xl/sharedStrings.xml";
        shared_string = read_shared_string(unziped_archive.find(shared_string_table_path)->second);


        auto workbook_relation_path = "xl/workbook.xml";
        sheet_relations = read_sheets_from_workbook(unziped_archive.find(workbook_relation_path)->second);

        for(int i = 0; i < sheet_relations.size(); i++)
        {
            auto cur_worksheet = new worksheet(this, get<1>(sheet_relations[i]), get<0>(sheet_relations[i]));
            _worksheets.emplace_back(cur_worksheet);
        }
    }
    string_view workbook::get_sheet_raw_content(uint32_t sheet_idx) const
    {
        auto sheet_path = "xl/worksheets/sheet" + to_string(sheet_idx) + ".xml";
        auto path_iter = file_contents.find(sheet_path);
        if(path_iter != file_contents.end())
        {
            return path_iter->second;
        }
        else
        {
			cout << "error load for sheet_path " << sheet_path;
			abort();
            return {};
        }
    }
    uint32_t workbook::get_worksheet_num() const
    {
        return sheet_relations.size();
    }
    const worksheet& workbook::get_worksheet(uint32_t sheet_idx) const
    {
        return *(_worksheets[sheet_idx]);
    }
    string_view workbook::get_workbook_name() const
    {
        return workbook_name;
    }
    workbook::workbook(string_view archive_filename)
    {
        file_contents = read_zip_to_memory(archive_filename.data());
        load_workbook_from_string(file_contents);
    }
    ostream& operator<<(ostream& output_stream, const workbook& in_workbook)
    {
        output_stream<<"workbook name:"<<in_workbook.get_workbook_name()<<endl;
        for(const auto& one_worksheet: in_workbook._worksheets)
        {
            output_stream<<*one_worksheet<<endl;
        }
		return output_stream;
    }

}