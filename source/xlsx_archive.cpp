#include <xlsx_archive.h>
#include <xlsx_utils.h>

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <iostream>
extern "C"
{
#include <miniz/miniz_zip.h>
}



namespace xlsx_reader
{
	using namespace std;
	using namespace tinyxml2;
	using namespace std::experimental::filesystem;
	archive::archive(const string& file_name)
	{
		cache_flag = false;
		path file_path(file_name);
		std::ifstream input_file_stream(file_path, std::ios::binary);
		input_file_stream.seekg(0, std::ios::end);
		string cur_all_content;
		cur_all_content.reserve(input_file_stream.tellg());
		input_file_stream.seekg(0, std::ios::beg);
		cur_all_content.assign(std::istreambuf_iterator<char>(input_file_stream), std::istreambuf_iterator<char>());
		std::cout << "miniz version " << MZ_VERSION << std::endl;
		std::cout << "string size " << cur_all_content.size() << " file path " << current_path() << std::endl;
		mz_zip_archive cur_archive;

		memset(&cur_archive, 0, sizeof(cur_archive));

		int status = mz_zip_reader_init_mem(&cur_archive, static_cast<const void*>(cur_all_content.c_str()), cur_all_content.size(), 0);
		if (!status)
		{
			std::cerr << "invalid zip file " << file_path << " status " << cur_archive.m_last_error << std::endl;
			valid_flag = false;
			clear_resource();
			return;
		}

		for (int i = 0; i < (int)mz_zip_reader_get_num_files(&cur_archive); i++)
		{
			mz_zip_archive_file_stat cur_file_stat;
			if (!mz_zip_reader_file_stat(&cur_archive, i, &cur_file_stat))
			{
				std::cerr << "mz_zip_reader_file_stat failed" << std::endl;
				valid_flag = false;
				clear_resource();
				return;
			}
			//std::cout << " Filename " << cur_file_stat.m_filename << "comment " << cur_file_stat.m_comment <<
			//	" uncompressed size " << cur_file_stat.m_uncomp_size << " compressed size " << cur_file_stat.m_comp_size
			//	<< " is dir " << mz_zip_reader_is_file_a_directory(&cur_archive, i) << std::endl;

			size_t cur_uncom_size = 0;
			void* p = mz_zip_reader_extract_file_to_heap(&cur_archive, cur_file_stat.m_filename, &cur_uncom_size, 0);
			if (!p)
			{
				std::cerr << "memory allocation fail for file " << cur_file_stat.m_filename << std::endl;
				valid_flag = false;
				clear_resource();
				return;
			}
			valid_flag = true;
			//std::cout << "success extracted file " << cur_file_stat.m_filename << std::endl;
			char* new_p = (char*)p;
			archive_file_buffers.push_back(p);
			archive_content[string(cur_file_stat.m_filename)] = string_view(static_cast<char*>(p), cur_file_stat.m_uncomp_size);
		}
		mz_zip_reader_end(&cur_archive);
	}
	shared_ptr<XMLDocument> archive::get_xml_document(const string& doc_path)
	{
		if (cache_flag)
		{
			auto doc_iter = xml_content.find(doc_path);
			if (doc_iter != xml_content.end())
			{
				return doc_iter->second;
			}
			else
			{
				auto content_iter = archive_content.find(doc_path);
				if (content_iter != archive_content.end())
				{
					auto current_ptr = make_shared<XMLDocument>();
					xml_content[doc_path] = current_ptr;
					current_ptr->Parse(content_iter->second.data(), content_iter->second.size());
					return current_ptr;
				}
				else
				{
					return nullptr;
				}
			}
		}
		else
		{
			auto content_iter = archive_content.find(doc_path);
			if (content_iter != archive_content.end())
			{
				auto current_ptr = make_shared<XMLDocument>();
				current_ptr->Parse(content_iter->second.data(), content_iter->second.size());
				return current_ptr;
			}
			else
			{
				return nullptr;
			}
		}
		
	}
	vector<sheet_desc> archive::get_all_sheet_relation()
	{
		auto workbook_relation_path = "xl/workbook.xml";
		auto cur_doc = get_xml_document(workbook_relation_path);
		vector<sheet_desc> all_sheets;
		auto workbook_node = cur_doc->FirstChildElement("workbook");
		auto sheets_node = workbook_node->FirstChildElement("sheets");
		auto sheets_begin = sheets_node->FirstChildElement("sheet");
		while (sheets_begin)
		{
			string current_sheet_name(sheets_begin->Attribute("name"));
			string current_sheet_id(sheets_begin->Attribute("sheetId"));
			string current_relation_id(sheets_begin->Attribute("r:id"));
			all_sheets.emplace_back(make_tuple(current_sheet_name, stoi(current_sheet_id), current_relation_id));
			sheets_begin = sheets_begin->NextSiblingElement("sheet");
		}
		return all_sheets;
	}

	vector<string> archive::get_shared_string()
	{
		auto shared_string_table_path = "xl/sharedStrings.xml";
		auto cur_shared_doc = get_xml_document(shared_string_table_path);
		vector<string> all_share_strings;
		auto share_total_node = cur_shared_doc->FirstChildElement("sst");
		auto share_string_begin = share_total_node->FirstChildElement("si");
		while (share_string_begin)
		{
			auto current_value = share_string_begin->FirstChildElement("t")->GetText();
			// cout << "value " << current_value << " has type " << current_type << endl;
			if (current_value)
			{
				all_share_strings.emplace_back(current_value);
			}
			else
			{
				all_share_strings.emplace_back(string());
			}
			share_string_begin = share_string_begin->NextSiblingElement("si");
		}
		return all_share_strings;
	}
	bool archive::get_cache_mode() const
	{
		return cache_flag;
	}
	void archive::set_cache_mode(bool enable_cache)
	{
		if (cache_flag == enable_cache)
		{
			return;
		}
		cache_flag = enable_cache;
		if (!cache_flag)
		{
			xml_content.clear();
		}
	}
	void archive::clear_resource()
	{
		for(auto i: archive_file_buffers)
		{
			free(i);
		}
		archive_file_buffers.clear();
		xml_content.clear();
	}
	bool archive::is_valid() const
	{
		return valid_flag;
	}
	archive::~archive()
	{
		clear_resource();
	}
}