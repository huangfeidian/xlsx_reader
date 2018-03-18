#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>


using relation_desc = std::tuple<std::string, std::string, std::string>;
using sheet_desc = std::tuple<std::string, std::string, std::string>;
enum class cell_type
{
	empty,
	boolean,
	date,
	inline_string,
	number,
	shared_string,
	formula_string,
	error
};

class cell_raw_value
{
public:
	const cell_type cur_type;
	const double numerical_value;
	const string text_value;
	
	cell_raw_value(int _shared_idx) :
		shared_idx(_shared_idx), is_shared(true), simple_value(""), simple_value_type("")
	{

	}
	cell_raw_value(const std::string& _simple_value, const std::string& _simple_value_type) :
		shared_idx(-1), is_shared(false), simple_value(_simple_value), simple_value_type(_simple_value_type)
	{

	}
};
class cell_final_value
{
public:
	const string& value;
	const string& value_type;
	cell_final_value(const string& _value, const string& _value_type) :
		value(_value), value_type(_value_type)
	{

	}
};