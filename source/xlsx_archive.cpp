#include <xlsx_archive.h>
#include <xlsx_utils.h>
#include <miniz.h>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <iostream>

namespace xlsx_reader
{
    using namespace std;
    using namespace tinyxml2;
    using namespace std::experimental::filesystem;
    archive::archive(const string& file_name)
    {
        path file_path(file_name);
        std::ifstream input_file_stream(file_path, std::ios::binary);
        input_file_stream.seekg(0, std::ios::end);
        string cur_all_content;
        cur_all_content.reserve(input_file_stream.tellg());
        input_file_stream.seekg(0, std::ios::beg);
        cur_all_content.assign(std::istreambuf_iterator<char>(input_file_stream), std::istreambuf_iterator<char>());
        std::cout << "minic version " << MZ_VERSION << std::endl;
        std::cout << "string size " << cur_all_content.size() << "file path " << current_path() << std::endl;
        mz_zip_archive cur_archive;

        memset(&cur_archive, 0, sizeof(cur_archive));

        int status = mz_zip_reader_init_mem(&cur_archive, static_cast<const void*>(cur_all_content.c_str()), cur_all_content.size(), 0);
        if (!status)
        {
            std::cerr << "invalid zip file " << file_path << " status " << cur_archive.m_last_error << std::endl;
            return;
        }

        for (int i = 0; i < (int)mz_zip_reader_get_num_files(&cur_archive); i++)
        {
            mz_zip_archive_file_stat cur_file_stat;
            if (!mz_zip_reader_file_stat(&cur_archive, i, &cur_file_stat))
            {
                std::cerr << "mz_zip_reader_file_stat failed" << std::endl;
                return;
            }
            std::cout << " Filename " << cur_file_stat.m_filename << "comment " << cur_file_stat.m_comment <<
                " uncompressed size " << cur_file_stat.m_uncomp_size << " compressed size " << cur_file_stat.m_comp_size
                << " is dir " << mz_zip_reader_is_file_a_directory(&cur_archive, i) << std::endl;

            size_t cur_uncom_size = 0;
            void* p = mz_zip_reader_extract_file_to_heap(&cur_archive, cur_file_stat.m_filename, &cur_uncom_size, 0);
            if (!p)
            {
                std::cerr << "memory allocation fail for file " << cur_file_stat.m_filename << std::endl;
                return;
            }
            std::cout << "success extracted file " << cur_file_stat.m_filename << std::endl;
            char* new_p = (char*)p;
            archive_file_buffers.push_back(p);
            archive_content[string(cur_file_stat.m_filename)] = string_view(static_cast<char*>(p), cur_file_stat.m_uncomp_size);
        }
        mz_zip_reader_end(&cur_archive);
    }
    const XMLDocument* archive::get_xml_document(const string& doc_path)
    {
        auto doc_iter = xml_content.find(doc_path);
        if(doc_iter != xml_content.end())
        {
            return doc_iter->second.get();
        }
        else
        {
            auto content_iter = archive_content.find(doc_path);
            if(content_iter != archive_content.end())
            {
                auto current_unique_ptr = make_unique<XMLDocument>();
                auto current_raw_ptr = current_unique_ptr.get();
                xml_content[doc_path] = move(current_unique_ptr);
                current_raw_ptr->Parse(content_iter->second.data(), content_iter->second.size());
                return current_raw_ptr;
            }
            else
            {
                return nullptr;
            }
        }
    }
    archive::~archive()
    {
        for(auto i: archive_file_buffers)
        {
            free(i);
        }
        archive_file_buffers.clear();
    }
}