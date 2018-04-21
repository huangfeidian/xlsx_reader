#pragma once
#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, std::string> read_zip_to_memory(const std::string& file_path);
