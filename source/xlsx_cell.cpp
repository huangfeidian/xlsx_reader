#include <xlsx_cell.h>
#include <xlsx_types.h>
#include <iostream>
namespace xlsx_reader
{
	using namespace std;
	cell_value::cell_value(uint32_t row, uint32_t column)
	{
		_row = row;
		_column = column;
	}
	cell::cell(uint32_t in_row, uint32_t in_column)
		:cell_value(in_row, in_column)
	{

	}
	void cell::set_value(bool _value)
	{
		_type = cell_type::number_bool;
		bool_v = _value;
	}
	void cell::set_value(uint32_t _value)
	{
		_type = cell_type::number_u32;
		int_u32_v = _value;
	}
	void cell::set_value(int32_t _value)
	{
		_type = cell_type::number_32;
		int_32_v = _value;
	}
	void cell::set_value(uint64_t _value)
	{
		_type = cell_type::number_u64;
		int_u64_v = _value;
	}
	void cell::set_value(int64_t _value)
	{
		_type = cell_type::number_64;
		int_64_v = _value;
	}
	void cell::set_value(float _value)
	{
		_type = cell_type::number_float;
		float_v = _value;
	}
	void cell::set_value(double _value)
	{
		_type = cell_type::number_double;
		double_v = _value;
	}
	void cell::set_value(string_view _value)
	{
		_type = cell_type::inline_string;
		_text = _value;
	}
	void cell::set_value(const date& _value)
	{
		_type = cell_type::date;
		double_v = _value.to_number(calendar::windows_1900);
	}
	void cell::set_value(const datetime& _value)
	{
		_type = cell_type::datetime;
		double_v = _value.to_number(calendar::windows_1900);
	}
	void cell::set_value(const time& _value)
	{
		_type = cell_type::time;
		double_v = _value.to_number();
	}
	uint32_t cell::get_row() const
	{
		return _row;
	}
	uint32_t cell::get_column() const
	{
		return _column;
	}
	pair<uint32_t, uint32_t> cell::get_row_column() const
	{
		return make_pair(_row, _column);
	}
	cell_type cell::get_type() const
	{
		return _type;
	}
	bool cell::operator==(const cell& rhs) const
	{
		if(_type != rhs._type || _text != rhs._text || double_v != rhs.double_v)
		{
			return false;
		}
		else
		{
			return true;
		}

	}
	cell& cell::operator=(const cell& rhs)
	{
		_type = rhs._type;
		_text = rhs._text;
		_row = rhs._row;
		_column = rhs._column;
		double_v = rhs.double_v;
		return *this;
	}
	template<>
	bool cell::expect_value<bool>() const
	{
		return bool_v;
	}
	template<>
	uint32_t cell::expect_value<uint32_t>() const
	{
		return int_u32_v;
	}
	template<>
	int32_t cell::expect_value<int32_t>() const
	{
		return int_32_v;
	}
	template<>
	uint64_t cell::expect_value<uint64_t>() const
	{
		return int_u64_v;
	}
	template<>
	int64_t cell::expect_value<int64_t>() const
	{
		return int_64_v;
	}
	template<>
	float cell::expect_value<float>() const
	{
		return float_v;
	}
	template<>
	double cell::expect_value<double>() const
	{
		return double_v;
	}
	template<>
	string_view cell::expect_value<string_view>() const
	{
		return _text;
	}


	string cell::to_string() const
	{
		switch (_type)
		{
		case cell_type::empty:
			return "";
		case cell_type::date:
			return date::from_number(double_v, calendar::windows_1900).to_string();
		case cell_type::time:
			return time::from_number(double_v).to_string();
		case cell_type::datetime:
			return datetime::from_number(double_v, calendar::windows_1900).to_string();
		case cell_type::number_32:
			return std::to_string(int_32_v);
		case cell_type::number_64:
			return std::to_string(int_64_v);
		case cell_type::number_u32:
			return std::to_string(int_u32_v);
		case cell_type::number_u64:
			return std::to_string(int_u64_v);
		case cell_type::number_float:
			return std::to_string(float_v);
		case cell_type::number_double:
			return std::to_string(double_v);
		case cell_type::error:
			return error_to_string(static_cast<cell_error>(int_u32_v));
		case cell_type::shared_string:
			return string(_text);
		case cell_type::formula_string:
			return string(_text);
		case cell_type::inline_string:
			return string(_text);
		case cell_type::number_bool:
			return bool_v ? "TRUE" : "FALSE";
		}
		return "";
	};
	ostream& operator<<(ostream& output_stream, const cell& in_cell)
	{
		output_stream<<"row:"<<in_cell._row<<" column:"<<in_cell._column<<" value:"<<in_cell.to_string()<<endl;
		return output_stream;
	}
	void cell::infer_value(string_view _value)
	{
		_text = _value;
		if(_value.empty())
		{
			return;
		}
		if(_value.front() == '=' && _value.size() > 1)
		{
			from_formula(_value.substr(1));
			return;
		}
		if(_value.front()=='#' && _value.size() > 1)
		{
			from_error(_value.substr(1));
			return;
		}
		auto percentage = cast_numeric(_value);
		if(percentage)
		{
			double_v = percentage.value();
			_type = cell_type::number_double;
			return;
		}
		auto cur_time = cast_time(_value);
		if(cur_time)
		{
			_type = cell_type::number_double;
			double_v = cur_time.value().to_number();
			return;
		}
	}
	void cell::from_formula(string_view _value)
	{
		_type = cell_type::formula_string;
	}

	void cell::from_error(string_view _value)
	{
		_type = cell_type::error;
		int_u32_v = static_cast<uint32_t>(error_from_string(string(_value)));
	}
}
