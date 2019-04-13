#include <string_table.h>
#include <cstdlib>

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
		for (const auto& one_long_str : long_strings)
		{
			if (source_str.data() >= one_long_str.data() && source_str.data() < one_long_str.data() + one_long_str.size())
			{
				return true;
			}
		}
		return false;
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
		auto size = other.size();
		if (size >= long_string_size)
		{
			// long string
			auto new_str_ptr = new char[size];
			std::copy(other.data(), other.data() + size, new_str_ptr);
			auto result = std::string_view(new_str_ptr, size);
			str_set.insert(result);
			long_strings.emplace_back(result);
			return result;
		}
		else
		{
			// short string
			bool should_alloc = false;
			if (buffers.size())
			{
				const auto& buffer_end = buffers.back();
				if (buffer_size - buffer_end.second < size)
				{
					should_alloc = true;
				}
			}
			else
			{
				should_alloc = true;
			}

			if(should_alloc)
			{
#ifdef _MSC_VER
				auto new_buffer = _aligned_malloc(buffer_size, buffer_size);
#else
				auto new_buffer = std::aligned_alloc(buffer_size, buffer_size);
#endif
				buffer_set.insert(reinterpret_cast<std::size_t>(new_buffer));
			}
			auto& the_end_buffer = buffers.back();
			std::copy(other.data(), other.data() + other.size(), the_end_buffer.first + the_end_buffer.second);
			auto result = std::string_view(the_end_buffer.first + the_end_buffer.second, size);
			the_end_buffer.second += size;
			str_set.insert(result);
			return result;

		}
	}
	std::string_view string_table::add(const std::string& other)
	{
		return add(std::string_view(other.data(), other.size()));
	}
	string_table::~string_table()
	{
		for (const auto& one_buffer : buffers)
		{
#ifdef _MSC_VER
			_aligned_free(one_buffer.first);
#else
			std::free(one_buffer);
#endif
		}
	}
}