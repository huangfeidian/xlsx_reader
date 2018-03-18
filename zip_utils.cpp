#include "zip_utils.h"
#include "miniz.h"
#include <filesystem>
#include <fstream>
#include <streambuf>
#include <iostream>

#include <vector>
#include <sstream>
using namespace std;
using namespace std::experimental::filesystem;
std::unordered_map<std::string, std::string> read_zip_to_memory(const std::string& file_name)
{
	unordered_map<string, string> result;
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
		return result;
	}

	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&cur_archive); i++)
	{
		mz_zip_archive_file_stat cur_file_stat;
		if (!mz_zip_reader_file_stat(&cur_archive, i, &cur_file_stat))
		{
			std::cerr << "mz_zip_reader_file_stat failed" << std::endl;
			return result;
		}
		std::cout << " Filename " << cur_file_stat.m_filename << "comment " << cur_file_stat.m_comment <<
			" uncompressed size " << cur_file_stat.m_uncomp_size << " compressed size " << cur_file_stat.m_comp_size
			<< " is dir " << mz_zip_reader_is_file_a_directory(&cur_archive, i) << std::endl;

		size_t cur_uncom_size = 0;
		void* p = mz_zip_reader_extract_file_to_heap(&cur_archive, cur_file_stat.m_filename, &cur_uncom_size, 0);
		if (!p)
		{
			std::cerr << "memory allocation fail for file " << cur_file_stat.m_filename << std::endl;
			return result;
		}
		std::cout << "success extracted file " << cur_file_stat.m_filename << std::endl;
		char* new_p = (char*)p;
		string output_buffer(new_p, new_p + cur_file_stat.m_uncomp_size);

		result[string(cur_file_stat.m_filename)] = output_buffer;
		free(p);
	}
	mz_zip_reader_end(&cur_archive);
	return result;

}