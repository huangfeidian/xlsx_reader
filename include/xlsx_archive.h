#pragma once


#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include <tinyxml2.h>

#include "xlsx_utils.h"
#include "memory_arena.h"

namespace spiritsaway::xlsx_reader
{
	class archive
	{
	private:
		std::unordered_map<std::string, std::string_view> archive_content;
		std::unordered_map<std::string, std::shared_ptr<tinyxml2::XMLDocument>> xml_content;
		std::vector<void*> archive_file_buffers;
		bool valid_flag = false;
		bool cache_flag = false;
		void clear_resource();
	public:
		archive(const std::string& file_path);
		archive& operator=(const archive& rhs) = delete;
		archive(const archive& rhs) = delete;
		std::shared_ptr<tinyxml2::XMLDocument> get_xml_document(const std::string& doc_path);
		std::vector<sheet_desc> get_all_sheet_relation();
		std::vector<std::string> get_shared_string(); 
		void get_shared_string_view(spiritsaway::memory::arena& string_arena, std::vector<std::string_view>& view_vec);
		void clear_xml_document_cache();
		bool is_valid() const;
		bool get_cache_mode() const;
		void set_cache_mode(bool enable_cache);
		~archive();
	};
}