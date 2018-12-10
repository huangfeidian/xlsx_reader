#pragma once
#include <xlsx_utils.h>
#include <tinyxml2.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
namespace xlsx_reader
{
	class archive
	{
	private:
		std::unordered_map<std::string, std::string_view> archive_content;
		std::unordered_map<std::string, std::unique_ptr<tinyxml2::XMLDocument>> xml_content;
		std::vector<void*> archive_file_buffers;
		bool valid_flag = false;
		void clear_resource();
	public:
		archive(const std::string& file_path);
		archive& operator=(const archive& rhs) = delete;
		archive(const archive& rhs) = delete;
		const tinyxml2::XMLDocument* get_xml_document(const std::string& doc_path);
		std::vector<sheet_desc> get_all_sheet_relation();
		std::vector<std::string_view> get_shared_string(); 
		bool is_valid() const;
		~archive();
	};
}