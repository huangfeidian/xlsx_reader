#include <xlsx_utils.h>
#include <tinyxml2.h>
#include <iostream>
#include <tuple>
#include <vector>
#include <cmath>
#include <charconv>

namespace spiritsaway::xlsx_reader
{

	using namespace std;
	using namespace tinyxml2;

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
		uint32_t max_col = 26 + 26 * 26 + 26 * 26 * 26 + 1;
		if (column_index < min_col || column_index > max_col)
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
	std::string_view strip_blank(std::string_view input)
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
			return std::string_view();
		}
		else
		{
			return input.substr(left, right - left);
		}
	}
}