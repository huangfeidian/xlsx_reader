#include <xlsx_utils.h>
#include <tinyxml2.h>
#include <iostream>
#include <tuple>
#include <vector>
#include <cmath>
#include <charconv>

namespace xlsx_reader
{

using namespace std;
using namespace tinyxml2;
optional<double> cast_numeric(string_view s)
{
	char* result_end = static_cast<char*>(nullptr);
	string current_str(s);
	auto cast_result = strtod(&(*current_str.cbegin()), &result_end);
	if (result_end != &current_str.back() + 1)
	{
		return {};
	}
	else
	{
		return cast_result;
	}
}
optional<int> cast_int(string_view s)
{

	char* result_end = static_cast<char*>(nullptr);
	string current_str(s);
	auto cast_result = strtol(&(*current_str.cbegin()), &result_end, 10);
	if (result_end != &current_str.back() + 1)
	{
		return {};
	}
	else
	{
		return cast_result;
	}
}
uint32_t column_index_from_string(const string& column_string)
{

	if (column_string.length() > 3 || column_string.empty())
	{
		return 0;
	}

	uint32_t column_index = 0;
	int place = 1;

	for (int i = static_cast<int>(column_string.length()) - 1; i >= 0; i--)
	{
		if (!std::isalpha(column_string[static_cast<std::size_t>(i)]))
		{
			return 0;
		}

		auto char_index = std::toupper(column_string[static_cast<std::size_t>(i)]) - 'A';

		column_index += static_cast<uint32_t>((char_index + 1) * place);
		place *= 26;
	}

	return column_index;
}
pair<uint32_t, uint32_t> row_column_tuple_from_string(const string& tuple_string)
{
	//sample input A1
	uint32_t column_char_size = 0;
	while (isalpha(tuple_string[column_char_size]))
	{
		column_char_size++;
	}
	uint32_t column_value = column_index_from_string(tuple_string.substr(0, column_char_size));
	uint32_t row_value = stoi(tuple_string.substr(column_char_size));
	return make_pair(row_value, column_value);
}
std::string column_string_from_index(uint32_t column_index)
{
	// these indicies corrospond to A->ZZZ and include all allowed
	// columns
	uint32_t min_col = 1;
	uint32_t max_col = 26 + 26*26 + 26*26*26 + 1;
	if(column_index < min_col || column_index > max_col)
	{
		return "ZZZZ";
	}
	int temp = static_cast<int>(column_index);
	std::string column_letter = "";

	while (temp > 0)
	{
		int quotient = temp / 26, remainder = temp % 26;

		// check for exact division and borrow if needed
		if (remainder == 0)
		{
			quotient -= 1;
			remainder = 26;
		}

		column_letter = std::string(1, char(remainder + 64)) + column_letter;
		temp = quotient;
	}

	return column_letter;
}
string row_column_tuple_to_string(pair<uint32_t, uint32_t> row_column_tuple)
{
	return column_string_from_index(row_column_tuple.second) + std::to_string(row_column_tuple.first);
}
vector<string_view> split_string(string_view input_string, char sep)
// 根据分隔符号去分割内容
// 但是得处理一下内部括号的问题例如(1,2),(3,4) 
{
	vector<string_view> tokens;
	size_t start = 0;
	size_t end = 0;
	string tuple_str = "),(";
	tuple_str[1] = sep;

	if (input_string.find(tuple_str) == string_view::npos)
	{
		while ((end = input_string.find(sep, start)) != string_view::npos)
		{
			if (end != start)
			{
				tokens.emplace_back(input_string.substr(start, end - start));
			}
			start = end + 1;
		}
		if (end != start)
		{
			tokens.emplace_back(input_string.substr(start));
		}
		return tokens;
	}
	// 说明要进入递归模式
	int left_count = 0;
	for (int i = 0; i < input_string.size(); i++)
	{
		switch (input_string[i])
		{
		case ')':
			left_count--;
			if (left_count < 0)
			{
				return {};
			}
			if (left_count == 0)
			{
				tokens.push_back(input_string.substr(start, i - start + 1));
				start = i;
			}
			break;
		case '(':
			left_count++;
			break;
		default:
			if (left_count == 0 && input_string[i] != sep)
			{
				return {};
			}
		}
	}
	if (left_count > 0)
	{
		return {};
	}
	return tokens;
		
}
string_view strip_blank(std::string_view input)
{
	int left = 0;
	int right = input.size();
	while (left < right)
	{
		auto cur_char = input[left];
		if (cur_char == ' ' || cur_char == '\t' || cur_char == '\n')
		{
			left++;
		}
		else
		{
			break;
		}
	}
	while (left < right)
	{
		
		auto cur_char = input[right - 1];
		if (cur_char == ' ' || cur_char == '\t' || cur_char == '\n')
		{
			right--;
		}
		else
		{
			break;
		}
	}
	if (left == right)
	{
		return string_view();
	}
	else
	{
		return input.substr(left, right - left);
	}
}
template<>
optional<bool> cast_string_view<bool>(string_view _text)
{
	if (_text == "0")
	{
		return false;
	}
	else if (_text == "1")
	{
		return true;
	}
	else
	{
		return nullopt;
	}
}
template<>
optional<uint32_t> cast_string_view<uint32_t>(string_view _text)
{
	uint32_t result;
	if (auto[p, ec] = std::from_chars(_text.data(), _text.data() + _text.size(), result); ec == std::errc())
	{
		return result;
	}
	return nullopt;
}
template<>
optional<int> cast_string_view<int>(string_view _text)
{
	int result;
	if (auto[p, ec] = std::from_chars(_text.data(), _text.data() + _text.size(), result); ec == std::errc())
	{
		return result;
	}
	return nullopt;
}
template<>
optional<uint64_t> cast_string_view<uint64_t>(string_view _text)
{
	uint64_t result;
	if (auto[p, ec] = std::from_chars(_text.data(), _text.data() + _text.size(), result); ec == std::errc())
	{
		return result;
	}
	return nullopt;
}
template<>
optional<int64_t> cast_string_view<int64_t>(string_view _text)
{
	int64_t result;
	if (auto[p, ec] = std::from_chars(_text.data(), _text.data() + _text.size(), result); ec == std::errc())
	{
		return result;
	}
	return nullopt;
}
template<>
optional<float> cast_string_view<float>(string_view _text)
{
	float result;
	if (auto[p, ec] = std::from_chars(_text.data(), _text.data() + _text.size(), result); ec == std::errc())
	{
		return result;
	}
	return nullopt;
}
template<>
optional<double> cast_string_view<double>(string_view _text)
{
	double result;
	if (auto[p, ec] = std::from_chars(_text.data(), _text.data() + _text.size(), result); ec == std::errc())
	{
		return result;
	}
	return nullopt;
}
}