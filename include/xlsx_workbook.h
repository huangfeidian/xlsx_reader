#pragma once
#include <xlsx_worksheet.h>
#include <memory>
namespace xlsx_reader
{
    class workbook
    {
    public:
        std::vector<std::unique_ptr<worksheet>> _worksheets;
        std::vector<tuple<std::string, std::uint32_t, std::string>> sheet_relations; 
        std::vector<std::string_view> shared_string;
        const worksheet& get_worksheet() const;
        std::uint32_t get_worksheet_num() const;
        std::string_view workbook_name;
        std::string_view get_workbook_name() const;
        workbook(std::string_view archive_content);
        workbook(std::string_view archive_filename);
    private:
        std::unordered_map<std::string, std::string> file_contents;
        void load_workbook_from_string(std::string_view _input_sting);
        worksheet* load_worksheet(std::string_view _input_string);
    }
}