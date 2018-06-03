#pragma once
#include <xlsx_cell_extend.h>
#include <nlohmann/json.hpp>
namespace xlsx_reader{
    using json = nlohmann::json;
    using namespace std;
	// 当前的json 并不支持从string_view转换 所以目前需要手工处理
    void to_json(json& j, const extend_node_type_descriptor& cur_type);
    
    void to_json(json& j, const extend_node_value& cur_value);
}