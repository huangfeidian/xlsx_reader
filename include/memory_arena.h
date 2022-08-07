#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
namespace spiritsaway::memory
{

	constexpr std::uint32_t align_to(std::uint32_t pre_size, std::uint32_t align_size)
	{
		if ((pre_size & (align_size - 1)) != 0)
		{
			pre_size = (pre_size ^ (align_size - 1)) + align_size;
		}
		return pre_size;
	}
	class arena
	{
		const std::uint32_t buffer_page_size;
		std::vector<std::pair<char*, std::uint32_t>> buffers;
		std::uint32_t cur_buffer_used = 0;
	public:
		arena(std::uint32_t page_size)
			:buffer_page_size(std::max(1024u, align_to(page_size, 1024u)))
		{

		}
		template <typename T>
		T* get(std::uint32_t count)
		{
			if (count == 0)
			{
				return nullptr;
			}
			return reinterpret_cast<T*>(get(sizeof(T) * count, alignof(T)));
		}
		char* get(std::uint32_t m_size, std::uint32_t align_size)
		{
			m_size = align_to(m_size, align_size);
			//for big page
			if (m_size >= buffer_page_size)
			{
				auto result = static_cast<char*>(malloc(m_size));
				if (buffers.empty())
				{
					buffers.push_back(std::make_pair(result, m_size));
					cur_buffer_used = buffer_page_size;
					return result;
				}
				else
				{
					auto cur_buffer_size = buffers.size();
					buffers.push_back(std::make_pair(result, m_size));
					std::swap(buffers[cur_buffer_size], buffers[cur_buffer_size - 1]);
					return result;
				}
			}
			// for small page
			cur_buffer_used = align_to(cur_buffer_used, align_size);
			if (buffers.empty())
			{
				auto new_buffer_p = static_cast<char*>(malloc(buffer_page_size));
				buffers.push_back(std::make_pair(new_buffer_p, buffer_page_size));

			}
			if (cur_buffer_used + m_size > buffer_page_size)
			{
				auto new_buffer_p = static_cast<char*>(malloc(buffer_page_size));
				cur_buffer_used = m_size;
				buffers.push_back(std::make_pair(new_buffer_p, buffer_page_size));
				return new_buffer_p;

			}
			else
			{
				auto result = cur_buffer_used;
				cur_buffer_used += m_size;
				auto cur_buffer_p = buffers.back();
				return cur_buffer_p.first + result;
			}
		}
		std::uint32_t consumption() const
		{
			std::uint32_t result = 0;
			for (const auto& one_page : buffers)
			{
				result += one_page.second;
			}
			return result;
		}
		void drop()
		{
			for (auto one_buffer : buffers)
			{
				free(one_buffer.first);
			}
			buffers.clear();
			cur_buffer_used = 0;
		}
		~arena()
		{
			drop();
			return;
		}
	};
}