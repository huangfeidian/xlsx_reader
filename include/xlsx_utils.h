#pragma once
#include <vector>
#include <string>
#include <xlsx_types.h>
#include <map>

namespace xlsx_reader
{
    std::vector<relation_desc> read_relations_from_content(std::string_view input_string);
    std::vector<sheet_desc> read_sheets_from_workbook(std::string_view input_string);
    std::vector<std::string_view> read_shared_string(std::string_view input_string);
    std::unordered_map<std::string, std::string> read_zip_to_memory(const std::string& file_path);
    std::vector<std::vector<cell*>> load_sheet_from_string(std::string_view input_string, const std::vector<std::string_view>& shared_string_table);
    std::string column_index_to_string(uint32_t col_idx);
    uint32_t column_index_from_string(const std::string& reference_string);
}

