#include <string_table.h>

namespace xlsx_reader
{
	string_table& string_table::get_instance()
	{
		string_table the_table;
		return the_table;
	}
	bool string_table::contains(std::string_view source_str) const
	{
		auto aligned_str = (reinterpret_cast<std::size_t>(source_str.data()) / string_table::buffer_size) * string_table::buffer_size;
		if (buffer_set.count(aligned_str))
		{
			return true;
		}
		else
		{
			return false;
		}

	}
	std::string_view string_table::add(std::string_view other)
	{
		if (contains(other))
		{
			return other;
		}
		auto str_iter = str_set.find(other);
		if (str_iter != str_set.end())
		{
			return *str_iter;
		}
		return *str_iter;
	}
}