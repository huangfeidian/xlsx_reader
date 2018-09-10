#pragma once
#include <xlsx_cell_extend.h>
#include <xlsx_worksheet.h>
#include <xlsx_workbook.h>
#include <nlohmann/json.hpp>

namespace xlsx_reader{
    using json = nlohmann::json;
    class typed_header
    {
    public:
        const extend_node_type_descriptor* type_desc;
        std::string_view header_name;
        std::string_view header_comment;
        typed_header(const extend_node_type_descriptor* in_type_desc, std::string_view in_header_name, std::string_view in_header_comment);
        friend std::ostream& operator<<(std::ostream& output_stream, const typed_header& in_typed_header);
        friend void to_json(json& j, const typed_header& cur_typed_header);
    };

    class typed_worksheet: public worksheet
    {
        // 这里我们默认第一行是id 第二行是类型 第三行是注释 第四行开始是数据
    public:
        typed_worksheet(const std::vector<cell>& all_cells, std::uint32_t in_sheet_id, std::string_view in_sheet_name, const workbook<typed_worksheet>* in_workbook);
        const std::map<std::uint32_t, const extend_node_value*>& get_typed_row(std::uint32_t) const;
        const typed_cell* get_typed_cell(std::uint32_t row_idx, std::uint32_t column_idx) const;
        friend std::ostream& operator<<(std::ostream& output_stream, const typed_worksheet& in_worksheet);
        friend void to_json(json& j, const typed_worksheet& cur_worksheet);
		template <typename T> friend class workbook;
        virtual ~typed_worksheet();
        const std::vector<typed_header>& get_typed_headers();
        std::uint32_t get_index_for_numeric(const extend_node_value& key);
        const workbook<typed_worksheet>* get_workbook() const;
    protected:
        std::vector<typed_header> typed_headers;
        std::map<std::uint32_t, std::map<std::uint32_t, const typed_cell*>> typed_row_info;
        std::vector<typed_cell> typed_cells;
        std::unordered_map<const extend_node_value&, std::uint32_t, extend_node_value_hash> _indexes;
        void after_load_process();
    private:
        void convert_cell_to_typed_value();
        bool convert_typed_header();
        virtual int value_begin_row() const; // 获取数据开始的行号
    };
    
}