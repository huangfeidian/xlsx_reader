#pragma once
#include <xlsx_worksheet.h>

namespace xlsx_reader
{
    class workbook
    {
    public:
        std::vector<worksheet> _worksheets;
        std::unordered_map<std::string, std::uint32_t> _indexed_worksheet_names;
        const worksheet& get_worksheet() const;
        std::uint32_t get_worksheet_num() const;
        std::string_view workbook_name;
        std::string_view get_workbook_name() const;
        workbook(std::string_view in_workbook_name);
        std::string load_workbook_from_string(std::string_view _input_sting);
    }
}