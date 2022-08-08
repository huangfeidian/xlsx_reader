#pragma once
#include "xlsx_typed_cell.h"
#include "xlsx_worksheet.h"
#include "xlsx_workbook.h"
#include <functional>
#include <unordered_map>
#include <any_container/decode.h>

namespace spiritsaway::xlsx_reader{
	using namespace spiritsaway::container;
	class typed_header
	{
	public:
		std::shared_ptr<const typed_string_desc> type_desc;
		std::string_view header_name;
		std::string_view header_comment;
		typed_header(std::shared_ptr<const typed_string_desc> in_type_desc, std::string_view in_header_name, std::string_view in_header_comment);
		friend std::ostream& operator<<(std::ostream& output_stream, const typed_header& in_typed_header);
		
		bool operator==(const typed_header& other) const;
		
	};

	class typed_worksheet: public worksheet
	{
		// 这里我们默认第一行是id 第二行是类型 第三行是注释 第四行开始是数据

		
	public:
		typed_worksheet(const std::vector<cell>& all_cells, std::uint32_t in_sheet_id, std::string_view in_sheet_name, const workbook<typed_worksheet>* in_workbook);
		std::uint32_t get_cell_value_index_pos(std::uint32_t row_idx, std::uint32_t column_idx) const;
		const json& get_typed_cell_value(std::uint32_t row_idx, std::uint32_t column_idx) const;
		friend std::ostream& operator<<(std::ostream& output_stream, const typed_worksheet& in_worksheet);
		
		template <typename T> friend class workbook;
		virtual ~typed_worksheet();
		// 获取所有的表头数据
		const std::vector<typed_header>& get_typed_headers() const;
		const workbook<typed_worksheet>* get_workbook() const;
		// 根据表头名字返回列号 如果不存在则返回0
		std::uint32_t get_header_idx(std::string_view header_name) const;
		// 根据第一列的值来获取所属的行
		bool check_header_match(const std::unordered_map<std::string_view, const typed_header*>& other_headers, std::string_view index_column_name) const;



		std::vector<std::uint32_t> get_header_index_vector(const std::vector<std::string_view>& header_names) const;
	public:
		typed_worksheet(const typed_worksheet& other) = delete;
		typed_worksheet& operator=(const typed_worksheet& other) = delete;
		std::uint32_t memory_consumption() const;

	private:
		std::vector<typed_header> m_typed_headers;
		std::vector<json> m_cell_json_values;
		std::unordered_map<std::string_view, std::uint32_t> header_column_index;
		std::string after_load_process();
		std::vector<std::uint32_t> m_cell_value_indexes;
	public:
		const std::vector<typed_header>& typed_headers() const
		{
			return m_typed_headers;
		}
		const std::vector<json>& cell_json_values() const
		{
			return m_cell_json_values;
		}
		const std::vector<std::uint32_t>& cell_value_indexes() const
		{
			return m_cell_value_indexes;
		}
	private:
		std::string convert_cells_to_json();
		std::string convert_typed_header();
		
		template <typename T>
		std::optional<T> try_convert_cell(std::uint32_t row_idx, std::uint32_t column_idx) const
		{
			T result;
			const auto& cur_json = get_typed_cell_value(row_idx, column_idx);
			if (!spiritsaway::serialize::decode(cur_json, result))
			{
				return {};
			}
			else
			{
				return result;
			}

		}
		template<typename... args, std::size_t... arg_idx>
		std::tuple<std::
			optional<args>...> try_convert_row_impl(std::uint32_t row_index, const std::vector<std::uint32_t>& column_index, std::index_sequence<arg_idx...>) const
		{
			return std::make_tuple(try_convert_cell<args>(row_index, column_index[arg_idx])...);
		}
	public:
		virtual std::uint32_t value_begin_row() const ; // 获取数据开始的行号
		template<typename... args>
		std::tuple<std::optional<args>...> try_convert_row(std::uint32_t row_index, const std::vector<std::uint32_t>& column_index) const
		{
			if (row_index == 0 || row_index >= max_rows)
			{
				return std::make_tuple(std::optional<args>()...);
			}
			if (column_index.size() != sizeof...(args))
			{
				return std::make_tuple(std::optional<args>()...);
			}
			for (auto i : column_index)
			{
				if (i <= 0 || i >= m_typed_headers.size())
				{
					return std::make_tuple(std::optional<args>()...);
				}
			}
			return try_convert_row_impl<args...>(row_index, column_index, std::index_sequence_for<args...>());
		}

	};
	
	
}