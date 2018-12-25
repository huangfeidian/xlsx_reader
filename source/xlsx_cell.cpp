#include <xlsx_cell.h>
#include <iostream>
#include <charconv>
namespace xlsx_reader
{
	using namespace std;
	cell::cell(uint32_t row, uint32_t column, string_view in_text)
	:_row(row),
	_column(column),
	_text(in_text)
	{

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
	bool cell::operator==(const cell& rhs) const
	{
		if(_text !=rhs._text)
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
		_row = rhs._row;
		_column = rhs._column;
		_text = rhs._text;
		return *this;
	}
	template<>
	optional<bool> cell::expect_value<bool>() const
	{
		if(_text == "0")
		{
			return false;
		}
		else if(_text == "1")
		{
			return true;
		}
		else
		{
			return nullopt;
		}
	}
	template<>
	optional<uint32_t> cell::expect_value<uint32_t>() const
	{
		uint32_t result;
		if(auto [p, ec] = std::from_chars(_text.data(), _text.data()+_text.size(), result);ec == std::errc())
		{
			return result;
		}
		return nullopt;
	}
	template<>
	optional<int> cell::expect_value<int>() const
	{
		int result;
		if(auto [p, ec] = std::from_chars(_text.data(), _text.data()+_text.size(), result);ec == std::errc())
		{
			return result;
		}
		return nullopt;
	}
	template<>
	optional<uint64_t> cell::expect_value<uint64_t>() const
	{
		uint64_t result;
		if(auto [p, ec] = std::from_chars(_text.data(), _text.data()+_text.size(), result);ec == std::errc())
		{
			return result;
		}
		return nullopt;
	}
	template<>
	optional<int64_t> cell::expect_value<int64_t>() const
	{
		int64_t result;
		if(auto [p, ec] = std::from_chars(_text.data(), _text.data()+_text.size(), result);ec == std::errc())
		{
			return result;
		}
		return nullopt;
	}
	template<>
	optional<float> cell::expect_value<float>() const
	{
		float result;
		if(auto [p, ec] = std::from_chars(_text.data(), _text.data()+_text.size(), result);ec == std::errc())
		{
			return result;
		}
		return nullopt;
	}
	template<>
	optional<double> cell::expect_value<double>() const
	{
		double result;
		if(auto [p, ec] = std::from_chars(_text.data(), _text.data()+_text.size(), result);ec == std::errc())
		{
			return result;
		}
		return nullopt;
	}

	template<>
	optional<string_view> cell::expect_value<string_view>() const
	{
		return _text;
	}

	ostream& operator<<(ostream& output_stream, const cell& in_cell)
	{
		output_stream<<"row:"<<in_cell._row<<" column:"<<in_cell._column<<" value:"<<in_cell._text<<endl;
		return output_stream;
	}
	
}
