#pragma once
#include "xlsx_typed_cell.h"
#include "xlsx_worksheet.h"
#include "xlsx_workbook.h"
#include <functional>
#include <unordered_map>
namespace xlsx_reader{
	
	class typed_header
	{
	public:
		const typed_node_type_descriptor* type_desc;
		std::string_view header_name;
		std::string_view header_comment;
		typed_header(const typed_node_type_descriptor* in_type_desc, std::string_view in_header_name, std::string_view in_header_comment);
		friend std::ostream& operator<<(std::ostream& output_stream, const typed_header& in_typed_header);
		
		bool operator==(const typed_header& other) const;
	};

	class typed_worksheet: public worksheet
	{
		// 这里我们默认第一行是id 第二行是类型 第三行是注释 第四行开始是数据
	public:
		typed_worksheet(const std::vector<cell>& all_cells, std::uint32_t in_sheet_id, std::string_view in_sheet_name, const workbook<typed_worksheet>* in_workbook);
		const std::vector<typed_value>& get_typed_row(std::uint32_t) const;
		const typed_value* get_typed_cell_value(std::uint32_t row_idx, std::uint32_t column_idx) const;
		friend std::ostream& operator<<(std::ostream& output_stream, const typed_worksheet& in_worksheet);
		
		template <typename T> friend class workbook;
		virtual ~typed_worksheet();
		// 获取所有的表头数据
		const std::vector<const typed_header*>& get_typed_headers() const;
		const workbook<typed_worksheet>* get_workbook() const;
		// 根据表头名字返回列号 如果不存在则返回0
		std::uint32_t get_header_idx(std::string_view header_name) const;
		// 根据第一列的值来获取所属的行
		std::optional<std::uint32_t> get_indexed_row(const typed_value* first_row_value) const;
		const std::vector<typed_value>& get_ref_row(std::string_view sheet_name, const typed_value* first_row_value) const;

		const std::vector<std::vector<typed_value>>& get_all_typed_row_info() const;

		bool check_header_match(const std::unordered_map<std::string_view, const typed_header*>& other_headers, std::string_view index_column_name, const std::vector<std::string_view>& int_ref_headers, const std::vector<std::string_view>& string_ref_headers) const;
	public:
		std::vector<const typed_header*> typed_headers;
		std::unordered_map<const typed_value*, std::uint32_t, typed_value_hash, typed_value_ptr_equal> _indexes;
		std::unordered_map<std::string_view, std::uint32_t> header_column_index;
		void after_load_process();
		typed_worksheet(const typed_worksheet& other) = delete;
		typed_worksheet& operator=(const typed_worksheet& other) = delete;
		std::uint32_t memory_details() const;
	private:
		std::vector<std::vector<typed_value>> all_cell_values;
		void convert_cell_to_typed_value();
		bool convert_typed_header();
		virtual int value_begin_row() const; // 获取数据开始的行号
	};
	
}