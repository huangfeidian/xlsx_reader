#pragma once
#include <xlsx_worksheet.h>
#include <memory>
namespace xlsx_reader
{
    class workbook
    {
    public:
        std::vector<std::unique_ptr<worksheet>> _worksheets;
        std::vector<sheet_desc> sheet_relations; 
        std::vector<std::string_view> shared_string;
        const worksheet& get_worksheet(std::uint32_t sheet_idx) const;
        std::uint32_t get_worksheet_num() const;
        std::string_view workbook_name;
        std::string_view get_workbook_name() const;
        workbook(std::string_view archive_content);
        friend std::ostream& operator<<(std::ostream& output_stream, const workbook& in_workbook);
    private:
        std::unordered_map<std::string, std::string> file_contents;
        void load_workbook_from_string(const std::unordered_map<std::string, std::string>& unziped_archive);
        worksheet* load_worksheet(std::string_view _input_string);
        std::string_view get_sheet_raw_content(std::uint32_t sheet_idx) const;
        friend class worksheet;

	};
}