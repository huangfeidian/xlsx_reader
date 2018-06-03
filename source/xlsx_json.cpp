#include <xlsx_json.h>
namespace xlsx_reader{
    using json = nlohmann::json;
    using namespace std;
    void to_json(json& j, const extend_node_type_descriptor& cur_type)
    {
        static unordered_map<basic_node_type_descriptor, string_view> type_to_string = {
			{basic_node_type_descriptor::comment, "comment"},
			{basic_node_type_descriptor::date, "date"},
			{basic_node_type_descriptor::time, "time"},
			{basic_node_type_descriptor::datetime, "datetime"},
			{basic_node_type_descriptor::string, "string"},
			{basic_node_type_descriptor::number_bool, "bool"},
			{basic_node_type_descriptor::number_u32, "uint32"},
			{basic_node_type_descriptor::number_32, "int32"},
			{basic_node_type_descriptor::number_64, "int64"},
			{basic_node_type_descriptor::number_u64, "uint64"},
			{basic_node_type_descriptor::number_float, "float"},
			{basic_node_type_descriptor::number_double, "double"},
        };
        auto temp_iter = type_to_string.find(cur_type._type);
        if(temp_iter != type_to_string.end())
        {
            j = json(string(temp_iter->second));
            return;
        }
        
        switch(cur_type._type)
        {
        case basic_node_type_descriptor::list:
            {
                auto temp_detail = std::get<extend_node_type_descriptor::list_detail_t>(cur_type._type_detail);
                json result_json;
				string sep_string = ",";
				sep_string[0] = std::get<2>(temp_detail);
                result_json["list"] = {{"type", *std::get<0>(temp_detail)}, {"seperator", sep_string}, {"size", std::get<1>(temp_detail)}};
                j = result_json;
                return; 
            }
        case basic_node_type_descriptor::tuple:
            {
                auto temp_detail = std::get<extend_node_type_descriptor::tuple_detail_t>(cur_type._type_detail);
                json result_json;
				json type_detail = json::array();
				for (const auto& i : temp_detail.first)
				{
					type_detail.push_back(*i);
				}
				string sep_string = ",";
				sep_string[0] = temp_detail.second;
				result_json["tuple"] = { {"type", type_detail}, {"seperator", sep_string} };
                j = result_json;
                return; 
            }
        case basic_node_type_descriptor::ref_id:
            {
                auto temp_detail = std::get<extend_node_type_descriptor::ref_detail_t>(cur_type._type_detail);
                if(!temp_detail.second.empty())
                {
                    json result_json;
                    result_json["ref"] = {string(temp_detail.first), string(temp_detail.second)};
                    j = result_json;
                    return;
                }
                else
                {
                    json result_json;
                    result_json["ref"] = {string(temp_detail.first)};
                    j = result_json;
                    return;
                }
            }
        default:
            j = nullptr;
            return;
        }
        return;

    }

    void to_json(json& j, const extend_node_value& cur_value)
    {
        switch(cur_value.type_desc->_type)
        {
        case basic_node_type_descriptor::comment:
            j = string(cur_value.v_text);
            return;
        case basic_node_type_descriptor::date:
            j = date::from_number(cur_value.v_int32, calendar::windows_1900).to_string();
            return;
        case basic_node_type_descriptor::time:
            j = time::from_number(cur_value.v_double).to_string();
            return;
        case basic_node_type_descriptor::number_bool:
            j = cur_value.v_bool;
            return;
        case basic_node_type_descriptor::number_32:
            j = cur_value.v_int32;
            return;
        case basic_node_type_descriptor::number_u32:
            j = cur_value.v_uint32;
            return;
        case basic_node_type_descriptor::number_64:
            j = cur_value.v_int64;
            return;

        case basic_node_type_descriptor::number_u64:
            j = cur_value.v_uint64;
            return;

        case basic_node_type_descriptor::number_float:
            j = cur_value.v_float;
            return;
        case basic_node_type_descriptor::number_double:
            j = cur_value.v_double;
            return;
        case basic_node_type_descriptor::ref_id:
            j = string(cur_value.v_text);
            return;
        case basic_node_type_descriptor::string:
            j = string(cur_value.v_text);
            return;
        case basic_node_type_descriptor::list:
            j = json::array();
            for(const auto& i: cur_value.v_list)
            {
                j.push_back(json(*i));
            }
            return;
        case basic_node_type_descriptor::tuple:
            j = json::array();
            for(const auto& i: cur_value.v_list)
            {
                j.push_back(json(*i));
            }
            return;
        default:
            j = nullptr;
            return;
        }
    }
}