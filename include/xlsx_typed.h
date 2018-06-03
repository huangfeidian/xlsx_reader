#pragma once
#include <xlsx_cell_extend.h>
#include <xlsx_worksheet.h>
#include <xlsx_workbook.h>

namespace xlsx_reader{
    class typed_header
    {
    public:
        const extend_node_type_descriptor* type_desc;
        std::string_view header_name;
        std::string_view header_comment;
        typed_header(const extend_node_type_descriptor* in_type_desc, std::string_view in_header_name, std::string_view in_header_comment);
        friend std::ostream& operator<<(std::ostream& output_stream, const typed_header& in_typed_header);
    };

    class typed_worksheet: public worksheet
    {
        // 这里我们默认第一行是id 第二行是类型 第三行是注释 第四行开始是数据
    public:
        const std::map<std::uint32_t, const extend_node_value*>& get_typed_row(std::uint32_t) const;
        const typed_cell* get_typed_cell(std::uint32_t row_idx, std::uint32_t column_idx) const;
        friend std::ostream& operator<<(std::ostream& output_stream, const typed_worksheet& in_worksheet);
    protected:
        std::vector<typed_header> typed_headers;
        std::map<std::uint32_t, std::map<std::uint32_t, const typed_cell*>> typed_row_info;
        void load_worksheet_from_string(std::string_view _input_string);
    private:
        void convert_cell_to_typed_value();
        bool convert_typed_header();
    };
    class typed_workbook: public workbook
    {
    public:
        typed_workbook(std::shared_ptr<archive> in_archive);
        friend std::ostream& operator<<(std::ostream& output_stream, const typed_workbook in_workbook);
        const typed_worksheet& get_typed_worksheet(std::uint32_t sheet_idx) const;
    };
}