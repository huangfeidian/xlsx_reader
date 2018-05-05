#include <xlsx_cell_extend.h>
#include <unordered_map>
#include <algorithm>
namespace {
    using namespace std;
    using namespace xlsx_reader;
    vector<string_view> parse_token_from_type_str(string_view type_string)
    {
        uint32_t begin_idx = 0;
        uint32_t end_idx = 0;
        vector<string_view> result;
        char cur_char = type_string[0];
        for(end_idx = 0; end_idx < type_string.size(); end_idx++)
        {
            cur_char = type_string[end_idx];
            if(cur_char == ',' || cur_char == '(' || cur_char == ')' || cur_char == ' ')
            {
                if(begin_idx != end_idx)
                {
                    result.emplace_back(type_string.data() + begin_idx, type_string.data() + end_idx);
                }
                else
                {
                    if(cur_char != ' ' && cur_char != ',')
                    {
                        result.emplace_back(type_string.data() + begin_idx, type_string.data() + end_idx + 1);
                    }
                }
                begin_idx = end_idx + 1;
            }
        }
        if(begin_idx != end_idx)
        {
            result.emplace_back(type_string.data() + begin_idx, type_string.data() + end_idx);
        }
        else
        {
            if(cur_char == ',')
            {
                return {};
            }
            if(cur_char != ' ')
            {
                result.emplace_back(type_string.data() + begin_idx, type_string.data() + end_idx + 1);
            }
        }
        return result;

    }
    vector<string_view> get_next_node(const vector<string_view>& tokens, size_t begin_idx)
    {
        vector<string_view> result;
        size_t unmatched = 0;
        while(begin_idx < tokens.size())
        {
            if(tokens[begin_idx] == string_view("("))
            {
                unmatched += 1;
            }
            else if(tokens[begin_idx] == string_view(")"))
            {
                
                unmatched -= 1;
            }
            result.push_back(tokens[begin_idx]);
            if(unmatched == 0)
            {
                if(tokens[begin_idx] == "list"sv || tokens[begin_idx] == "tuple"sv || tokens[begin_idx] == "ref"sv)
                {
                    begin_idx += 1;
                }
                else
                {
                    return result;
                }
            }
            else
            {
                begin_idx += 1;
            }
            
        }
        if(unmatched != 0)
        {
            return {};
        }
        else
        {
            return result;
        }
    }
    extend_node_type_descriptor* parse_type_from_tokens(vector<string_view> tokens)
    {
        static unordered_map<string_view, basic_node_type_descriptor> str_to_type_map = {
            {"comment", basic_node_type_descriptor::comment},
            {"date", basic_node_type_descriptor::date},
            {"time", basic_node_type_descriptor::time},
            {"datetime", basic_node_type_descriptor::datetime},
            {"str", basic_node_type_descriptor::string},
            {"bool", basic_node_type_descriptor::number_bool},
            {"uint", basic_node_type_descriptor::number_u32},
            {"int", basic_node_type_descriptor::number_32},
            {"uint64", basic_node_type_descriptor::number_u64},
            {"int64", basic_node_type_descriptor::number_64},
            {"float", basic_node_type_descriptor::number_float},
            {"double", basic_node_type_descriptor::number_double},
        };
        if(tokens.empty())
        {
            return nullptr;
        }
        if(tokens.size() == 1)
        {
            auto map_iter = str_to_type_map.find(tokens[0]);
            if(map_iter != str_to_type_map.end())
            {
                return new extend_node_type_descriptor(map_iter->second);
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            if(tokens[0] == string_view("ref"))
            {
                if(tokens.size() == 4)
                {
                    //ref(worksheet)
                    if(tokens[1] != string_view("(") || tokens[3] != string_view(")"))
                    {
                        return nullptr;
                    }
                    else
                    {
                        auto result = new extend_node_type_descriptor(basic_node_type_descriptor::ref_id);
                        result->_type_detail = make_pair(tokens[2], string_view(""));
                        return result;
                    }
                }
                else if(tokens.size() == 6)
                {
                    if(tokens[1] != string_view("(") || tokens[3] != string_view(",") || tokens[5] != string_view(")"))
                    {
                        return nullptr;
                    }
                    else
                    {
                        auto result = new extend_node_type_descriptor(basic_node_type_descriptor::ref_id);
                        result->_type_detail = make_pair(tokens[2], tokens[4]);
                        return result;
                    }
                }
                else
                {
                    return nullptr;
                }
            }
            else if(tokens[0] == string_view("list") || tokens[0] == string_view("tuple"))
            {

                if(tokens[1] != string_view("("))
                {
                    return nullptr;
                }
                if(tokens.size() < 5)
                {
                    return nullptr;
                }
                if(tokens.back() != string_view(")"))
                {
                    return nullptr;
                }
                vector<vector<string_view>> grouped_tokens;
                auto sub_token_begin_idx = 2;
                while(tokens[sub_token_begin_idx] != ")"sv)
                {
                    auto cur_sub_tokens = get_next_node(tokens, 2);
                    if(cur_sub_tokens.empty())
                    {
                        return nullptr;
                    }
                    sub_token_begin_idx += cur_sub_tokens.size();
                    if(sub_token_begin_idx >= tokens.size())
                    {
                        return nullptr;
                    }
                    grouped_tokens.push_back(move(cur_sub_tokens));
                }
                
                if(tokens[0] == "list"sv)
                {
                    char splitor = ',';
                    if(grouped_tokens.size() == 3)
                    {
                        auto splitor_info = grouped_tokens.back();
                        grouped_tokens.pop_back();
                        if(splitor_info.size() != 1)
                        {
                            return nullptr;
                        }
                        if(splitor_info[0].size() != 1)
                        {
                            return nullptr;
                        }
                        splitor = splitor_info[0][0];
                    }
                    if(grouped_tokens.size() != 2)
                    {
                        return nullptr;
                    }
                    auto temp_type_result = parse_type_from_tokens(grouped_tokens[0]);
                    if(!temp_type_result)
                    {
                        return nullptr;
                    }
                    if(grouped_tokens[1].size() != 1)
                    {
                        return nullptr;
                    }

                    auto repeat_times = cast_int(grouped_tokens[1][0]);
                    if(!repeat_times)
                    {
                        return nullptr;
                    }
                    else
                    {
                        return new extend_node_type_descriptor(make_tuple(temp_type_result, repeat_times.value(), splitor));
                    }

                }
                else
                {
                    //tuple(xx, xx)
                    vector<extend_node_type_descriptor*> type_vec;
                    char cur_splitor = ',';
                    auto splitor_info = grouped_tokens.back();
                    if(splitor_info.size() == 1 && splitor_info[0].size() == 1)
                    {
                        cur_splitor = splitor_info[0][0];
                        grouped_tokens.pop_back();
                    }
                    for(const auto & one_grouped_tokens: grouped_tokens)
                    {
                        auto temp_result = parse_type_from_tokens(one_grouped_tokens);
                        if(!temp_result)
                        {
                            return nullptr;
                        }
                        else
                        {
                            type_vec.push_back(temp_result);
                        }
                        
                    }
                    return new extend_node_type_descriptor(make_pair(type_vec, cur_splitor));
                }

            }
        }
    }
    extend_node_value* parse_value_with_type(const extend_node_type_descriptor* node_type, const cell_value& in_cell_value)
    {
        switch(node_type->_type)
        {
        case basic_node_type_descriptor::comment:
            return new extend_node_value();
        case basic_node_type_descriptor::date:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(static_cast<int>(in_cell_value.double_v));
        case basic_node_type_descriptor::time:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(in_cell_value.double_v);
        case basic_node_type_descriptor::datetime:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(in_cell_value.double_v);
        case basic_node_type_descriptor::string:
            return new extend_node_value(in_cell_value._text);
        case basic_node_type_descriptor::number_bool:
            return new extend_node_value(in_cell_value.bool_v);
        case basic_node_type_descriptor::number_32:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(static_cast<int>(in_cell_value.double_v));
        case basic_node_type_descriptor::number_u32:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(static_cast<uint32_t>(in_cell_value.double_v));
        case basic_node_type_descriptor::number_64:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(static_cast<int64_t>(in_cell_value.double_v));
        case basic_node_type_descriptor::number_u64:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(static_cast<uint64_t>(in_cell_value.double_v));
        case basic_node_type_descriptor::number_float:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(static_cast<float>(in_cell_value.double_v));
        case basic_node_type_descriptor::number_double:
            if(in_cell_value._type != cell_type::number_double)
            {
                return nullptr;
            }
            return new extend_node_value(in_cell_value.double_v);

        }
    }
    extend_node_value* parse_value_with_type(const extend_node_type_descriptor* node_type, string_view text)
    {
        
    }
}
namespace xlsx_reader{
    using namespace std;
    extend_node_type_descriptor* extend_node_value_constructor::parse_type(string_view type_string)
    {
        auto all_tokens = parse_token_from_type_str(type_string);
        return parse_type_from_tokens(all_tokens);
    }

}