#include <xlsx_worksheet.h>

namespace xlsx_reader {
    using namespace std;

    worksheet::worksheet(const workbook* in_workbook, uint32_t in_sheet_id, string_view in_sheet_name)
    : _workbook(in_workbook), _sheetId(in_sheeet_id)
    {
        return;
    }
    void load_worksheet_from_string(string_view input_string)
    {
        
    }
}