#pragma once
#include <vector>
#include <string>
#include "xlsx_types.h"

std::vector<relation_desc> read_relations_from_content(const std::string& input);
std::vector<sheet_desc> read_sheets_from_workbook(const std::string& input);
std::vector<cell_final_value> read_shared_strings(const std::string& input);
std::vector<std::vector<cell_raw_value>> read_worksheet(const std::string& input);
