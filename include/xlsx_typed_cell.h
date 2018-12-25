#pragma once
#include "xlsx_cell.h"
#include <string_view>
#include <cstdint>
#include <optional>
#include <memory>
#include <variant>
#include <string>
#include <vector>

namespace xlsx_reader
{
	template<typename T>
	struct is_tuple_impl : std::false_type {};

	template<typename... Ts>
	struct is_tuple_impl<std::tuple<Ts...>> : std::true_type {};

	template<typename T>
	struct is_tuple : is_tuple_impl<std::decay_t<T>> {};
	class cell_type_converter
	{
		// 用来处理类型转换
	};
	enum class basic_node_type_descriptor
	{
		comment,//COMMENT
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
	
	struct typed_node_type_descriptor
	{
		using ref_detail_t = std::tuple<std::string_view, std::string_view, std::string_view>;//这个字段只有在引用类型才会用到 workbook worksheet ref_type
		using list_detail_t = std::tuple<typed_node_type_descriptor*, std::uint32_t, char>;//这个字段只有在list类型的情况才会被使里面存的是list的成员信息 第二个分量是0的时候代表无限长度 detail_type length seperator
		using tuple_detail_t = std::pair<std::vector<typed_node_type_descriptor*>, char>;//这个字段只有在复合类型的情况才会被使里面存的是tuple的成员信息 <type1, type2, type3> seperator
		basic_node_type_descriptor _type;
		std::variant<ref_detail_t, tuple_detail_t, list_detail_t> _type_detail;
		typed_node_type_descriptor();
		typed_node_type_descriptor(basic_node_type_descriptor in_type);
		typed_node_type_descriptor(const tuple_detail_t& tuple_detail);
		typed_node_type_descriptor(const list_detail_t& list_detail);
		typed_node_type_descriptor(const ref_detail_t& ref_detail);
		friend std::ostream& operator<<(std::ostream& output_stream, const typed_node_type_descriptor& cur_node);
		static const typed_node_type_descriptor* get_basic_type_desc(basic_node_type_descriptor in_type);
		
		friend bool operator==(const typed_node_type_descriptor& cur, const typed_node_type_descriptor& other);
		friend bool operator!=(const typed_node_type_descriptor& cur, const typed_node_type_descriptor& other);
		std::optional<list_detail_t> get_list_detail_t() const;
		std::optional<ref_detail_t> get_ref_detail_t() const;
		std::optional<tuple_detail_t> get_tuple_detail_t() const;
		~typed_node_type_descriptor();
	};

	struct typed_value
	{
	public:
		const typed_node_type_descriptor* type_desc;
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
		std::vector<typed_value*> v_list;
		typed_value();
		typed_value(bool in_value);
		typed_value(std::uint32_t in_value);
		typed_value(std::int32_t in_value);
		typed_value(std::int64_t in_value);
		typed_value(std::uint64_t in_value);
		typed_value(std::string_view in_value);
		typed_value(float in_value);
		typed_value(double in_value);
		typed_value(const typed_node_type_descriptor* in_type_desc, std::vector<typed_value*> in_value);
		typed_value(const typed_node_type_descriptor* in_type_desc, std::string_view in_value);
		friend std::ostream& operator<<(std::ostream& output_stream, const typed_value& cur_node);
		
		friend bool operator==(const typed_value& cur, const typed_value& other);
		friend bool operator!=(const typed_value& cur, const typed_value& other);
		template <typename T> 
		std::optional<T> expect_simple_value() const;
		template <typename T>
		std::optional<T> expect_value() const;
		template <typename... args>
		std::optional<std::tuple<args ...>> expect_tuple_value() const;
		~typed_value();
	private:
		template<typename T> struct deduce_type{};
		template <typename T>
		auto expect_value_dispatch(deduce_type<T>) const
		{
			return expect_simple_value<T>();
		}
		template <typename... args>
		auto expect_value_dispatch(deduce_type<std::tuple<args...>>) const
		{
			return expect_tuple_value<args...>();
		}
	};

	template <>
	std::optional<std::uint32_t> typed_value::expect_simple_value<std::uint32_t>() const;
	template <>
	std::optional<std::int32_t> typed_value::expect_simple_value<std::int32_t>() const;
	template <>
	std::optional<std::int64_t> typed_value::expect_simple_value<std::int64_t>() const;
	template <>
	std::optional<std::uint64_t> typed_value::expect_simple_value<std::uint64_t>() const;
	template <>
	std::optional<float> typed_value::expect_simple_value<float>() const;
	template <>
	std::optional<double> typed_value::expect_simple_value<double>() const;
	template <>
	std::optional<bool> typed_value::expect_simple_value<bool>() const;
	template <>
	std::optional<std::string_view> typed_value::expect_simple_value<std::string_view>() const;

	template<typename... args, size_t... arg_idx>
	std::optional<std::tuple<args...>> get_tuple_value_from_vector(const std::vector<typed_value*>& v_list, std::index_sequence<arg_idx...>)
	{
		if(!(v_list[arg_idx] &&...))
		{
			return std::nullopt;
		}
		auto temp_result = std::make_tuple((*v_list[arg_idx]).expect_value<args>()...);

		if(!(std::get<arg_idx>(temp_result) &&...))
		{
			return std::nullopt;
		}
		return std::make_tuple(std::get<arg_idx>(temp_result).value()...);

	}

	template<typename... args>
	std::optional<std::tuple<args...>> typed_value::expect_tuple_value() const
	{
		if(v_list.size() == 0)
		{
			return std::nullopt;
		}
		for(const auto i: v_list)
		{
			if(!i)
			{
				return std::nullopt;
			}
		}
		auto the_tuple_size = sizeof...(args);
		if(v_list.size() != the_tuple_size)
		{
			return std::nullopt;
		}
		return get_tuple_value_from_vector<args...>(v_list, std::index_sequence_for<args...>{});

	}
	template <typename T>
	std::optional<T> typed_value::expect_value() const
	{
		return expect_value_dispatch(deduce_type<T>());
	}

	struct typed_value_hash
	{
		std::size_t operator()(const typed_value* s) const;
	};
	struct typed_value_ptr_equal
	{
		bool operator()(const typed_value* from, const typed_value* to) const;
	};
	class typed_cell
	{
	public:
		static const int row_begin = 1;
		static const int column_begin = 1;
		const typed_value* cur_typed_value;
		std::uint32_t _row;
		std::uint32_t _column;
		typed_cell(std::uint32_t in_row, std::uint32_t in_column, const typed_value* in_value);
		typed_cell(const typed_cell& other) = default;
		typed_cell& operator=(const typed_cell& other) = default;
		template <typename T> 
		std::optional<T> expect_value() const;
	};
	template <typename T>
	std::optional<T> typed_cell::expect_value() const
	{
		if(!cur_typed_value)
		{
			return std::nullopt;
		}
		else
		{
			return cur_typed_value->expect_value<T>();
		}
	}
	class typed_node_value_constructor
	{
	public:
		const typed_node_type_descriptor* type_desc;
		typed_node_value_constructor(std::string_view type_desc_text);
		const typed_cell* match_node(const cell* in_cell_value);

		static typed_node_type_descriptor* parse_type(std::string_view type_string);
		static typed_value* parse_value_with_type(const typed_node_type_descriptor* node_type, std::string_view text);
		static typed_cell* parse_node(const typed_node_type_descriptor* type_desc, const cell* in_cell_value);
		template <typename T>
		static typed_cell* prase_node_with_number(const typed_node_type_descriptor* type_desc, const cell* in_cell_value)
		{
			std::optional<T> result_opt = in_cell_value->expect_value<T>();
			if(!result_opt)
			{
				return nullptr;
			}
			return new typed_cell(in_cell_value->_row, in_cell_value->_column, new typed_value(result_opt.value()));
		}

	};

}