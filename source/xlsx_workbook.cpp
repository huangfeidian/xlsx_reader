#include <xlsx_workbook.h>
#include <xlsx_utils.h>
#include <tinyxml2.h>
namespace xlsx_reader
{
    using namespace std;
    using namespace tinyxml2;
    workbook load_workbook_from_string(unordered_map<string, string> unziped_archive)
    {
        auto shared_string_table_path = "xl/sharedStrings.xml";
        shared_string = read_shared_string(unziped_archive.find(shared_string_table_path).second);


        auto workbook_relation_path = "xl/workbook.xml";
        sheet_relations = read_sheets_from_workbook(unziped_archive.find(workbook_relation_path).second);

        for(int i = 0; i < sheet_relations.size(); i++)
        {
            auto cur_worksheet = new worksheet(&this, i, sheet_relations[i].get<0>());
            _worksheets.emplace_back(cur_worksheet);
        }


    }

}