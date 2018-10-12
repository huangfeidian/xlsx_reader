#pragma once
#include <xlsx_cell.h>
#include <string_view>
#include <cstdint>
#include <optional>
#include <memory>
#include <variant>
#include <string>
#include <nlohmann/json.hpp>

namespace xlsx_reader
{
	using json = nlohmann::json;
	class cell_type_converter
	{
		// 用来处理类型转换
	};
	enum class basic_node_type_descriptor
	{
		comment,//COMMENT
		date,//DATE
		time,//TIME
		datetime,//DATETIME
		string,//STRING
		number_bool,//BOOL
		number_u32,//UINT
		number_32,//INT
		number_u64,//LLINT
		number_64,///ULLINT
		number_float,//FLOAT
		number_double,//DOUBLE
		// 下面三个都是特殊
		ref_id,// 引用类型 其实就是一个字符串REF(WORKBOOK,WORKSHEET) OR REF(WORKSHEET)
		tuple,// 元组类型 TUPLE(FLOAT, STRING)
		list,//列表 LIST(FLOAT, 3) LIST(FLOAT, 0) LIST(TUPLE(FLOAT, STRING), 2) LIST(LIST(FLOAT, 3), 3)

	};
	
	struct extend_node_type_descriptor
	{
		using ref_detail_t = std::tuple<std::string_view, std::string_view, std::string_view>;//这个字段只有在引用类型才会用到 workbook worksheet ref_type
		using list_detail_t = std::tuple<extend_node_type_descriptor*, std::uint32_t, char>;//这个字段只有在list类型的情况才会被使里面存的是list的成员信息 第二个分量是0的时候代表无限长度 detail_type length seperator
		using tuple_detail_t = std::pair<std::vector<extend_node_type_descriptor*>, char>;//这个字段只有在复合类型的情况才会被使里面存的是tuple的成员信息 <type1, type2, type3> seperator
		basic_node_type_descriptor _type;
		std::variant<ref_detail_t, tuple_detail_t, list_detail_t> _type_detail;
		extend_node_type_descriptor();
		extend_node_type_descriptor(basic_node_type_descriptor in_type);
		extend_node_type_descriptor(const tuple_detail_t& tuple_detail);
		extend_node_type_descriptor(const list_detail_t& list_detail);
		extend_node_type_descriptor(const ref_detail_t& ref_detail);
		friend std::ostream& operator<<(std::ostream& output_stream, const extend_node_type_descriptor& cur_node);
		static const extend_node_type_descriptor* get_basic_type_desc(basic_node_type_descriptor in_type);
		friend void to_json(json& j, const extend_node_type_descriptor& cur_extend_node_type_descriptor);
		friend bool operator==(const extend_node_type_descriptor& cur, const extend_node_type_descriptor& other);
		friend bool operator!=(const extend_node_type_descriptor& cur, const extend_node_type_descriptor& other);
		std::optional<list_detail_t> get_list_detail_t() const;
		std::optional<ref_detail_t> get_ref_detail_t() const;
		std::optional<tuple_detail_t> get_tuple_detail_t() const;
	};

	struct extend_node_value
	{
	public:
		const extend_node_type_descriptor* type_desc;
		std::string_view v_text;
		union {
			bool v_bool;
			std::uint32_t v_uint32; 
			std::int32_t v_int32;
			std::uint64_t v_uint64;
			std::int64_t v_int64;
			float v_float;
			double v_double; //for time datetime date double
		};
		std::vector<extend_node_value*> v_list;
		extend_node_value();
		extend_node_value(bool in_value);
		extend_node_value(std::uint32_t in_value);
		extend_node_value(std::int32_t in_value);
		extend_node_value(std::int64_t in_value);
		extend_node_value(std::uint64_t in_value);
		extend_node_value(std::string_view in_value);
		extend_node_value(float in_value);
		extend_node_value(double in_value);
		extend_node_value(const extend_node_type_descriptor* in_type_desc, std::vector<extend_node_value*> in_value);
		extend_node_value(const extend_node_type_descriptor* in_type_desc, std::string_view in_value);
		friend std::ostream& operator<<(std::ostream& output_stream, const extend_node_value& cur_node);
		friend void to_json(json& j, const extend_node_value& cur_extend_node_value);
		friend bool operator==(const extend_node_value& cur, const extend_node_value& other);
		friend bool operator!=(const extend_node_value& cur, const extend_node_value& other);
		template <typename T>
		std::optional<T> get_value() const;

	};
	template <>
	std::optional<std::uint32_t> extend_node_value::get_value<std::uint32_t>() const;
	template <>
	std::optional<std::int32_t> extend_node_value::get_value<std::int32_t>() const;
	template <>
	std::optional<std::int64_t> extend_node_value::get_value<std::int64_t>() const;
	template <>
	std::optional<std::uint64_t> extend_node_value::get_value<std::uint64_t>() const;
	template <>
	std::optional<float> extend_node_value::get_value<float>() const;
	template <>
	std::optional<double> extend_node_value::get_value<double>() const;
	template <>
	std::optional<bool> extend_node_value::get_value<bool>() const;
	template <>
	std::optional<std::string_view> extend_node_value::get_value<std::string_view>() const;

	struct extend_node_value_hash
	{
		std::size_t operator()(const extend_node_value& s);
	};
	class typed_cell
    {
	public:
        const extend_node_value* cur_typed_value;
		std::uint32_t _row;
		std::uint32_t _column;
        typed_cell(std::uint32_t in_row, std::uint32_t in_column, const extend_node_value* in_value);
        typed_cell(const typed_cell& other) = default;
        typed_cell& operator=(const typed_cell& other) = default;
		template <typename T> 
		std::optional<T> expect_value() const;
    };
	class extend_node_value_constructor
	{
	public:
		const extend_node_type_descriptor* type_desc;
		extend_node_value_constructor(std::string_view type_desc_text);
		const typed_cell* match_node(const cell* in_cell_value);

		static extend_node_type_descriptor* parse_type(std::string_view type_string);
		static extend_node_value* parse_value_with_type(const extend_node_type_descriptor* node_type, std::string_view text);
		static typed_cell* parse_node(const extend_node_type_descriptor* type_desc, const cell* v_cell);

	};

}