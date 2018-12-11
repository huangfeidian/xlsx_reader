#include <xlsx_cell_extend.h>
#include <unordered_map>
#include <algorithm>
#include <xlsx_utils.h>
#include <numeric>
#include <functional>
namespace {
	using namespace std;
	string_view strip_parenthesis(string_view input_string)
	{
		// input (xxx) return xxx
		// if not(xxx) return empty
		auto left_idx = input_string.find_first_of("(");
		if(left_idx == string_view::npos)
		{
			return {};
		}
		input_string.remove_prefix(left_idx + 1);
		auto right_idx = input_string.find_last_of(")");
		if(right_idx == string_view::npos)
		{
			return {};
		}
		
		input_string.remove_suffix(input_string.size() - right_idx);
		return input_string;
	}
	string accumulate_string(const vector<string>& str_list)
	{
		// return accumulate(str_list.cbegin(), str_list.cend(), "", [](const string& a, const string& b)
		// {
		// 	return a + b;
		// }
		string result_str;
		int total_size = 0;
		for(const auto& i : str_list)
		{
			total_size += i.size();
		}
		result_str.reserve(total_size);
		for (const auto& i : str_list)
		{
			result_str += i;
		}
		return result_str;
	}
}
namespace xlsx_reader{
	using namespace std;
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
					if (cur_char != ' ' && cur_char != ',')
					{
						result.emplace_back(type_string.data() + begin_idx, end_idx - begin_idx);
						result.emplace_back(type_string.data() + end_idx, 1);
					}
					else
					{
						result.emplace_back(type_string.data() + begin_idx, end_idx - begin_idx);
					}
					
				}
				else
				{
					if(cur_char != ' ' && cur_char != ',')
					{
						result.emplace_back(type_string.data() + begin_idx, end_idx + 1 - begin_idx);
					}
				}
				begin_idx = end_idx + 1;
			}
		}
		if(begin_idx != end_idx)
		{
			result.emplace_back(type_string.data() + begin_idx, end_idx - begin_idx);
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
			{"uint32", basic_node_type_descriptor::number_u32},
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
				// ref(worksheet, int) ref(worksheet, str) ref(workbook, worksheet, str) ref(workbook, worksheet, int)
				if(tokens[1] != string_view("(") || tokens.back() != string_view(")"))
				{
					return nullptr;
				}
				auto cur_type_desc = tokens[tokens.size() -2];
				if(cur_type_desc != "str"sv && cur_type_desc != "int"sv)
				{
					return nullptr;
				}
				string_view cur_worksheet;
				string_view cur_workbook;
				if(tokens.size() == 5)
				{
					cur_worksheet = tokens[2];
				}
				else if(tokens.size() == 6)
				{
					cur_workbook = tokens[2];
					cur_worksheet = tokens[3];
				}
				else
				{
					return nullptr;
				}
				auto result = new extend_node_type_descriptor(basic_node_type_descriptor::ref_id);
				result->_type_detail = make_tuple(cur_workbook, cur_worksheet, cur_type_desc);
				return result;
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
					auto cur_sub_tokens = get_next_node(tokens, sub_token_begin_idx);
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
						// list(type, len, seperator)
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
					// list(type, len)
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
					//tuple(xx, xx, seperator)
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
		return nullptr;
	}
	extend_node_value* parse_value_with_type(const extend_node_type_descriptor* node_type, const cell_value& in_cell_value)
	{
		switch(node_type->_type)
		{
		case basic_node_type_descriptor::comment:
			return nullptr;
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
		default:
			return nullptr;

		}
	}
	extend_node_value* extend_node_value_constructor::parse_value_with_type(const extend_node_type_descriptor* node_type, string_view text)
	{
		text.remove_prefix(min(text.find_first_not_of(" "), text.size()));
		text.remove_suffix(text.size() - min(text.find_last_not_of(" ") + 1, text.size()));
		if(text.size() == 0)
		{
			return nullptr;
		}
		auto current_double_value = cast_numeric(text);
		switch(node_type->_type)
		{
			case basic_node_type_descriptor::comment:
				return nullptr;
			case basic_node_type_descriptor::string:
				return new extend_node_value(text);
			case basic_node_type_descriptor::date:
				if (current_double_value.has_value())
				{
					return new extend_node_value(static_cast<uint32_t>(current_double_value.value()));
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::time:
				if (current_double_value.has_value())
				{
					return new extend_node_value(current_double_value.value());
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::datetime:

				if (current_double_value.has_value())
				{
					return new extend_node_value(current_double_value.value());
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::number_bool:
				if (text == "TRUE"sv)
				{
					return new extend_node_value(true);
				}
				else if (text == "FALSE"sv)
				{
					return new extend_node_value(false);
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::number_32:
				if (current_double_value.has_value())
				{
					return new extend_node_value(static_cast<int32_t>(current_double_value.value()));
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::number_u32:
				if (current_double_value.has_value())
				{
					return new extend_node_value(static_cast<uint32_t>(current_double_value.value()));
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::number_64:
				if (current_double_value.has_value())
				{
					return new extend_node_value(static_cast<int64_t>(current_double_value.value()));
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::number_u64:
				if (current_double_value.has_value())
				{
					return new extend_node_value(static_cast<uint64_t>(current_double_value.value()));
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::number_float:
				if (current_double_value.has_value())
				{
					return new extend_node_value(static_cast<float>(current_double_value.value()));
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::number_double:
				if (current_double_value.has_value())
				{
					return new extend_node_value(current_double_value.value());
				}
				else
				{
					return nullptr;
				}
			case basic_node_type_descriptor::ref_id:
			{
				auto cur_ref_detail = std::get<extend_node_type_descriptor::ref_detail_t>(node_type->_type_detail);
				if (std::get<2>(cur_ref_detail) == "str"sv)
				{
					return new extend_node_value(node_type, text);
				}
				else
				{
					return new extend_node_value(static_cast<uint64_t>(current_double_value.value()));
				}
			}
			case basic_node_type_descriptor::tuple:
			{
				auto cur_tuple_detail = std::get<extend_node_type_descriptor::tuple_detail_t>(node_type->_type_detail);
				char sep = cur_tuple_detail.second;
				auto type_list = cur_tuple_detail.first;
				text = strip_parenthesis(text);
				auto tokens = split_string(text, sep);
				if (tokens.size() != type_list.size())
				{
					return nullptr;
				}
				vector<extend_node_value *> sub_values;
				for (int i = 0; i < type_list.size(); i++)
				{
					auto cur_value = parse_value_with_type(type_list[i], tokens[i]);
					if (!cur_value)
					{
						return nullptr;
					}
					sub_values.push_back(cur_value);
				}
				return new extend_node_value(node_type, sub_values);
			}
			case basic_node_type_descriptor::list:
			{
				auto cur_list_detail = std::get<extend_node_type_descriptor::list_detail_t>(node_type->_type_detail);
				uint32_t list_size = std::get<uint32_t>(cur_list_detail);
				char sep = std::get<char>(cur_list_detail);
				auto unit_type = std::get<extend_node_type_descriptor *>(cur_list_detail);
				text = strip_parenthesis(text);
				auto tokens = split_string(text, sep);
				vector<extend_node_value *> sub_values;
				if (list_size == 0)
				{
					for (auto one_token : tokens)
					{
						auto cur_value = parse_value_with_type(unit_type, one_token);
						if (!cur_value)
						{
							return nullptr;
						}
						sub_values.push_back(cur_value);
					}
				}
				else
				{
					if (tokens.size() != list_size)
					{
						return nullptr;
					}
					sub_values.reserve(list_size);
					for (auto one_token : tokens)
					{
						auto cur_value = parse_value_with_type(unit_type, one_token);
						if (!cur_value)
						{
							return nullptr;
						}
						sub_values.push_back(cur_value);
					}
				}
				return new extend_node_value(node_type, sub_values);
			}
			default:
				return nullptr;
		}
	}
	optional<extend_node_type_descriptor::list_detail_t> extend_node_type_descriptor::get_list_detail_t() const
	{
		if(_type != basic_node_type_descriptor::list)
		{
			return nullopt;
		}
		else
		{
			return std::get<extend_node_type_descriptor::list_detail_t>(_type_detail);
		}
	}

	optional<extend_node_type_descriptor::ref_detail_t> extend_node_type_descriptor::get_ref_detail_t() const
	{
		if(_type != basic_node_type_descriptor::ref_id)
		{
			return nullopt;
		}
		else
		{
			return std::get<extend_node_type_descriptor::ref_detail_t>(_type_detail);
		}
	}

	optional<extend_node_type_descriptor::tuple_detail_t> extend_node_type_descriptor::get_tuple_detail_t() const
	{
		if(_type != basic_node_type_descriptor::tuple)
		{
			return nullopt;
		}
		else
		{
			return std::get<extend_node_type_descriptor::tuple_detail_t>(_type_detail);
		}
	}

	extend_node_type_descriptor::extend_node_type_descriptor()
	{
		_type = basic_node_type_descriptor::comment;
	}
	extend_node_type_descriptor::extend_node_type_descriptor(basic_node_type_descriptor in_type)
	{
		_type = in_type;
	}
	extend_node_type_descriptor::extend_node_type_descriptor(const extend_node_type_descriptor::tuple_detail_t& tuple_detail):
		_type(basic_node_type_descriptor::tuple), _type_detail(tuple_detail)
	{

	}
	extend_node_type_descriptor::extend_node_type_descriptor(const extend_node_type_descriptor::list_detail_t& list_detail):
		_type(basic_node_type_descriptor::list), _type_detail(list_detail)
	{

	}
	extend_node_type_descriptor::extend_node_type_descriptor(const extend_node_type_descriptor::ref_detail_t& ref_detail)
		:_type(basic_node_type_descriptor::ref_id), _type_detail(ref_detail)
	{

	}
	extend_node_type_descriptor::~extend_node_type_descriptor()
	{
		if(_type == basic_node_type_descriptor::tuple)
		{
			auto temp_detail = get<extend_node_type_descriptor::tuple_detail_t>(_type_detail);
			for(auto k : temp_detail.first)
			{
				if(k)
				{
					k->~extend_node_type_descriptor();
				}
			}
		}
		else if(_type == basic_node_type_descriptor::list)
		{
			auto temp_detail = get<extend_node_type_descriptor::list_detail_t>(_type_detail);
			auto k = get<0>(temp_detail);
			if(k)
			{
				k->~extend_node_type_descriptor();
			}
		}
	}
	ostream& operator<<(ostream& output_stream, const extend_node_type_descriptor& cur_type)
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
			{basic_node_type_descriptor::list, "list"},
			{basic_node_type_descriptor::ref_id, "ref"},
			{basic_node_type_descriptor::tuple, "tuple"},
		};
		auto temp_iter = type_to_string.find(cur_type._type);
		if(temp_iter == type_to_string.end())
		{
			return output_stream<<"invalid";
		}
		output_stream<<temp_iter->second;
		if(cur_type._type == basic_node_type_descriptor::ref_id)
		{
			auto temp_detail = std::get<extend_node_type_descriptor::ref_detail_t>(cur_type._type_detail);
			string_view cur_workbook, cur_worksheet, cur_ref_type;
			std::tie(cur_workbook, cur_worksheet, cur_ref_type) = temp_detail;
			if(cur_workbook.empty())
			{
				output_stream<<"(" << cur_worksheet<<", "<<cur_ref_type<<")";
			}
			else
			{
				output_stream<<"(" <<cur_workbook <<", "<< cur_worksheet<<", "<<cur_ref_type<<")";
			}
			return output_stream;
		}
		else if(cur_type._type == basic_node_type_descriptor::tuple)
		{
			auto temp_detail = std::get<extend_node_type_descriptor::tuple_detail_t>(cur_type._type_detail);
			output_stream<<"(";

			for(int i = 0; i< temp_detail.first.size(); i++)
			{
				output_stream<<*temp_detail.first[i];
				if(i != temp_detail.first.size() - 1)
				{
					output_stream<<",";
				}
			}
			if(temp_detail.second != ',')
			{
				output_stream<<", "<< temp_detail.second; 
			}
			output_stream<<")";
			return output_stream;
		}
		else if(cur_type._type == basic_node_type_descriptor::list)
		{
			auto temp_detail = std::get<extend_node_type_descriptor::list_detail_t>(cur_type._type_detail);
			output_stream<<"("<<*std::get<0>(temp_detail)<<", "<<std::get<1>(temp_detail);
			auto sep = std::get<2>(temp_detail);
			if(sep != ',')
			{
				output_stream<<", "<<sep;
			}
			output_stream<<")";
			return output_stream;
		}
		else
		{
			return output_stream;
		}
	}
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
            j = json(temp_iter->second);
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
                string_view cur_workbook, cur_worksheet, cur_ref_type;
                if(!cur_workbook.empty())
                {
                    json result_json;
                    result_json["ref"] = {cur_worksheet, cur_ref_type};
                    j = result_json;
                    return;
                }
                else
                {
                    json result_json;
                    result_json["ref"] = {cur_workbook, cur_worksheet, cur_ref_type};
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
	bool operator==(const extend_node_type_descriptor& cur, const extend_node_type_descriptor& other)
	{
		if(cur._type != other._type)
		{
			return false;
		}
		switch(cur._type)
		{
			case basic_node_type_descriptor::comment:
			case basic_node_type_descriptor::date:
			case basic_node_type_descriptor::datetime:
			case basic_node_type_descriptor::string:
			case basic_node_type_descriptor::number_bool:
			case basic_node_type_descriptor::number_32:
			case basic_node_type_descriptor::number_64:
			case basic_node_type_descriptor::number_double:
			case basic_node_type_descriptor::number_float:
			case basic_node_type_descriptor::number_u32:
			case basic_node_type_descriptor::number_u64:
				return true;
			case basic_node_type_descriptor::list:
			{
				if(cur._type_detail.index() != other._type_detail.index())
				{
					return false;
				}
				auto cur_detail = std::get<extend_node_type_descriptor::list_detail_t>(cur._type_detail);
				auto other_detail = std::get<extend_node_type_descriptor::list_detail_t>(other._type_detail);
				if(std::get<1>(cur_detail) != std::get<1>(other_detail))
				{
					return false;
				}
				if(*std::get<0>(cur_detail) == *std::get<0>(other_detail))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
				
			case basic_node_type_descriptor::ref_id:
			{
				if(cur._type_detail.index() != other._type_detail.index())
				{
					return false;
				}
				auto cur_detail = std::get<extend_node_type_descriptor::ref_detail_t>(cur._type_detail);
				auto other_detail = std::get<extend_node_type_descriptor::ref_detail_t>(other._type_detail);
				return cur_detail == other_detail;
			}
				
			case basic_node_type_descriptor::tuple:
			{
				if(cur._type_detail.index() != other._type_detail.index())
				{
					return false;
				}
				auto cur_detail = std::get<extend_node_type_descriptor::tuple_detail_t>(cur._type_detail);
				auto other_detail = std::get<extend_node_type_descriptor::tuple_detail_t>(other._type_detail);
				if(cur_detail.first.size() != other_detail.first.size())
				{
					return false;
				}
				for(int i = 0; i < cur_detail.first.size(); i++)
				{
					if(*cur_detail.first[i] == *other_detail.first[i])
					{
						continue;
					}
					else
					{
						return false;
					}
				}
				return true;
			}
			default:
				return false;
		}
	}
	bool operator!=(const extend_node_type_descriptor& cur, const extend_node_type_descriptor& other)
	{
		return !(cur == other);
	}
	extend_node_type_descriptor* extend_node_value_constructor::parse_type(string_view type_string)
	{
		auto all_tokens = parse_token_from_type_str(type_string);
		return parse_type_from_tokens(all_tokens);
	}
	ostream& operator<<(ostream& output_stream, const extend_node_value& cur_value)
	{
		switch(cur_value.type_desc->_type)
		{
			case basic_node_type_descriptor::comment:
				return output_stream<<cur_value.v_text;
			case basic_node_type_descriptor::date:
				return output_stream<<date::from_number(cur_value.v_int32, calendar::windows_1900).to_string();
			case basic_node_type_descriptor::time:
				return output_stream<<time::from_number(cur_value.v_double).to_string();
			case basic_node_type_descriptor::datetime:
				return output_stream<<datetime::from_number(cur_value.v_double, calendar::windows_1900).to_string();
			case basic_node_type_descriptor::number_bool:
				return output_stream<<(cur_value.v_bool?"TRUE":"FALSE");
			case basic_node_type_descriptor::number_32:
				return output_stream<<cur_value.v_int32;
			case basic_node_type_descriptor::number_u32:
				return output_stream<<cur_value.v_uint32;
			case basic_node_type_descriptor::number_64:
				return output_stream<<cur_value.v_int64;
			case basic_node_type_descriptor::number_u64:
				return output_stream<<cur_value.v_uint64;
			case basic_node_type_descriptor::number_float:
				return output_stream<<cur_value.v_float;
			case basic_node_type_descriptor::number_double:
				return output_stream<<cur_value.v_double;
			case basic_node_type_descriptor::ref_id:
				return output_stream<<cur_value.v_text;
			case basic_node_type_descriptor::string:
				return output_stream<<cur_value.v_text;
			case basic_node_type_descriptor::list:
			{
				auto cur_list_detail = std::get<extend_node_type_descriptor::list_detail_t>(cur_value.type_desc->_type_detail);
				char sep = std::get<2>(cur_list_detail);
				output_stream<<"(";
				int cur_size = cur_value.v_list.size();
				for(int i = 0;i<cur_size; i++)
				{
					output_stream<<*cur_value.v_list[i];
					if(i != cur_size -1)
					{
						output_stream << sep ;
					}
				}
				return output_stream<<")";
			}
			case basic_node_type_descriptor::tuple:
			{
				auto cur_tuple_detail = std::get<extend_node_type_descriptor::tuple_detail_t>(cur_value.type_desc->_type_detail);
				char sep = cur_tuple_detail.second;
				output_stream<<"(";
				int cur_size = cur_value.v_list.size();
				for(int i = 0;i<cur_size; i++)
				{
					output_stream<<*cur_value.v_list[i];
					if(i != cur_size -1)
					{
						output_stream<<sep;
					}
				}
				return output_stream<<")";
			}
			default:
				return output_stream;

		}
	}
	
	void to_json(json& j, const extend_node_value& cur_value)
	{
		if (!cur_value.type_desc)
		{
			j = nullptr;
			return;
		}
		switch(cur_value.type_desc->_type)
        {
        case basic_node_type_descriptor::comment:
            j = cur_value.v_text;
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
            j = cur_value.v_text;
            return;
        case basic_node_type_descriptor::string:
            j = cur_value.v_text;
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
	const extend_node_type_descriptor* extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor in_type)
	{
		static vector<extend_node_type_descriptor> result = {
			extend_node_type_descriptor(basic_node_type_descriptor::comment),
			extend_node_type_descriptor(basic_node_type_descriptor::date),
			extend_node_type_descriptor(basic_node_type_descriptor::time),
			extend_node_type_descriptor(basic_node_type_descriptor::datetime),
			extend_node_type_descriptor(basic_node_type_descriptor::string),
			extend_node_type_descriptor(basic_node_type_descriptor::number_bool),
			extend_node_type_descriptor(basic_node_type_descriptor::number_u32),
			extend_node_type_descriptor(basic_node_type_descriptor::number_32),
			extend_node_type_descriptor(basic_node_type_descriptor::number_u64),
			extend_node_type_descriptor(basic_node_type_descriptor::number_64),
			extend_node_type_descriptor(basic_node_type_descriptor::number_float),
			extend_node_type_descriptor(basic_node_type_descriptor::number_double),
		};
		if (static_cast<uint32_t>(in_type) > static_cast<uint32_t>(basic_node_type_descriptor::number_double))
		{
			return &result[0];
		}
		else
		{
			return &result[static_cast<uint32_t>(in_type)];
		}
	}

	extend_node_value::extend_node_value()
		:type_desc(nullptr)
	{

	}
	extend_node_value::extend_node_value(bool in_value):
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::number_bool))
	{
		v_bool = in_value;
	}
	extend_node_value::extend_node_value(uint32_t in_value) :
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::number_u32))
	{
		v_uint32 = in_value;
	}
	extend_node_value::extend_node_value(int32_t in_value) :
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::number_32))
	{
		v_int32 = in_value;
	}
	extend_node_value::extend_node_value(uint64_t in_value) :
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::number_u64))
	{
		v_uint64 = in_value;
	}
	extend_node_value::extend_node_value(int64_t in_value) :
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::number_64))
	{
		v_int64 = in_value;
	}
	extend_node_value::extend_node_value(float in_value) :
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::number_float))
	{
		v_float = in_value;
	}
	extend_node_value::extend_node_value(double in_value) :
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::number_double))
	{
		v_double = in_value;
	}
	extend_node_value::extend_node_value(string_view in_value) :
		type_desc(extend_node_type_descriptor::get_basic_type_desc(basic_node_type_descriptor::string))
	{
		v_text = in_value;
	}
	extend_node_value::extend_node_value(const extend_node_type_descriptor* in_type_desc, string_view in_value):
		type_desc(in_type_desc)
	{
		v_text = in_value;
	}

	extend_node_value::extend_node_value(const extend_node_type_descriptor* in_type_desc, vector<extend_node_value*> in_value)
		: type_desc(in_type_desc), v_list(in_value)
	{
	}

	typed_cell::typed_cell(uint32_t in_row, uint32_t in_column, const extend_node_value* in_value):_row(in_row), _column(in_column), cur_typed_value(in_value)
	{

	}

	bool operator==(const extend_node_value& cur, const extend_node_value& other)
	{
		if(!cur.type_desc || ! other.type_desc)
		{
			return false;
		}
		if(*cur.type_desc!=(*other.type_desc))
		{
			return false;
		}
		switch(cur.type_desc->_type)
		{
		case basic_node_type_descriptor::comment:
        case basic_node_type_descriptor::string:
            return cur.v_text == other.v_text;
        case basic_node_type_descriptor::date:
        case basic_node_type_descriptor::datetime:
        case basic_node_type_descriptor::number_double:
			return cur.v_double == other.v_double;
        case basic_node_type_descriptor::number_32:
			return cur.v_int32 == other.v_int32;
        case basic_node_type_descriptor::number_u32:
            return cur.v_uint32 == other.v_uint32;
        case basic_node_type_descriptor::number_64:
            return cur.v_int64 == other.v_int64;
        case basic_node_type_descriptor::number_u64:
            return cur.v_uint64 == other.v_uint64;
        case basic_node_type_descriptor::number_bool:
            return cur.v_bool == other.v_bool;
        case basic_node_type_descriptor::number_float:
			return cur.v_float == other.v_float;
        case basic_node_type_descriptor::list:
        case basic_node_type_descriptor::tuple:
        case basic_node_type_descriptor::ref_id:
		{
			if (cur.v_list.size() != other.v_list.size())
			{
				return false;
			}
			auto cur_size = cur.v_list.size();
			for (int i = 0; i < cur_size; i++)
			{
				if (*cur.v_list[i] == *other.v_list[i])
				{
					continue;
				}
				else
				{
					return false;
				}
			}
			return true;
		}
			
        default:
            return false;
        }
	}
	bool operator!=(const extend_node_value& cur, const extend_node_value& other)
	{
		return !(cur == other);
	}

	size_t extend_node_value_hash::operator()(const extend_node_value* s) const
	{
		if (!s)
		{
			return 0;
		}
		switch(s->type_desc->_type)
		{
		case basic_node_type_descriptor::comment:
        case basic_node_type_descriptor::string:
            return std::hash<std::string_view>()(s->v_text);
        case basic_node_type_descriptor::date:
        case basic_node_type_descriptor::datetime:
        case basic_node_type_descriptor::number_double:
			return std::hash<double>()(s->v_double);
        case basic_node_type_descriptor::number_32:
			return std::hash<std::int32_t>()(s->v_int32);
        case basic_node_type_descriptor::number_u32:
            return std::hash<std::uint32_t>()(s->v_uint32);
        case basic_node_type_descriptor::number_64:
            return std::hash<std::int64_t>()(s->v_int64);
        case basic_node_type_descriptor::number_u64:
            return std::hash<std::uint64_t>()(s->v_uint64);
        case basic_node_type_descriptor::number_bool:
            return std::hash<bool>()(s->v_bool);
        case basic_node_type_descriptor::number_float:
			return std::hash<float>()(s->v_float);
        case basic_node_type_descriptor::list:
        case basic_node_type_descriptor::tuple:
        case basic_node_type_descriptor::ref_id:
		{
			auto cur_size = s->v_list.size();
			std::size_t result_hash = 0;
			for (const auto & i : s->v_list)
			{
				result_hash += operator()(i) / cur_size;
			}
			return result_hash;
		}
			
        default:
            return 0;
        }
	}
	bool extend_node_value_ptr_equal::operator()(const extend_node_value* from, const extend_node_value* to) const
	{
		if(from == to)
		{
			return true;
		}
		if(!from ||!to)
		{
			return false;
		}
		return (*from == *to);
	}
   typed_cell* extend_node_value_constructor::parse_node(const extend_node_type_descriptor* type_desc, const cell* in_cell_value)
    {
        if(!in_cell_value)
        {
            return nullptr;
        }
        switch(type_desc->_type)
        {
        case basic_node_type_descriptor::comment:
        case basic_node_type_descriptor::string:
            if(in_cell_value->_type != cell_type::inline_string && in_cell_value->_type != cell_type::shared_string)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column, new extend_node_value(in_cell_value->get_value<string_view>()));
        case basic_node_type_descriptor::date:
        case basic_node_type_descriptor::datetime:
        case basic_node_type_descriptor::number_double:
            if(in_cell_value->_type != cell_type::number_double)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column,new extend_node_value(in_cell_value->get_value<double>()));
        case basic_node_type_descriptor::number_32:
            if(in_cell_value->_type != cell_type::number_double)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column, new extend_node_value(in_cell_value->get_value<int>()));
        case basic_node_type_descriptor::number_u32:
            if(in_cell_value->_type != cell_type::number_double)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column,new extend_node_value(in_cell_value->get_value<uint32_t>()));
        case basic_node_type_descriptor::number_64:
            if(in_cell_value->_type != cell_type::number_double)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column,new extend_node_value(in_cell_value->get_value<int64_t>()));
        case basic_node_type_descriptor::number_u64:
            if(in_cell_value->_type != cell_type::number_double)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column,new extend_node_value(in_cell_value->get_value<uint64_t>()));
        case basic_node_type_descriptor::number_bool:
            if(in_cell_value->_type != cell_type::number_bool)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column,new extend_node_value(in_cell_value->get_value<bool>()));
        case basic_node_type_descriptor::number_float:
            if(in_cell_value->_type != cell_type::number_double)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column,new extend_node_value(in_cell_value->get_value<float>()));
        case basic_node_type_descriptor::list:
        case basic_node_type_descriptor::tuple:
        case basic_node_type_descriptor::ref_id:
            if(in_cell_value->_type != cell_type::inline_string && in_cell_value->_type != cell_type::shared_string)
            {
                return nullptr;
            }
            return new typed_cell(in_cell_value->_row, in_cell_value->_column,extend_node_value_constructor::parse_value_with_type(type_desc, in_cell_value->get_value<string_view>()));
        default:
            return nullptr;
        }
    }
	extend_node_value::~extend_node_value()
	{
		for(auto i: v_list)
		{
			if(i)
			{
				i->~extend_node_value();
			}
		}
		v_list.clear();
	}
	const typed_cell* extend_node_value_constructor::match_node(const cell* in_cell_value)
	{
		return extend_node_value_constructor::parse_node(type_desc, in_cell_value);
	} 
	template <>
	optional<std::uint32_t> extend_node_value::get_value<uint32_t>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::number_u32)
		{
			return nullopt;
		}
		else
		{
			return v_uint32;
		}
	}

	template <>
	optional<std::int32_t> extend_node_value::get_value<int32_t>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::number_32)
		{
			return nullopt;
		}
		else
		{
			return v_int32;
		}
	}

	template <>
	optional<std::int64_t> extend_node_value::get_value<int64_t>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::number_64)
		{
			return nullopt;
		}
		else
		{
			return v_int64;
		}
	}
	template <>
	optional<std::uint64_t> extend_node_value::get_value<uint64_t>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::number_u64)
		{
			return nullopt;
		}
		else
		{
			return v_uint64;
		}
	}
	template <>
	optional<bool> extend_node_value::get_value<bool>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::number_bool)
		{
			return nullopt;
		}
		else
		{
			return v_bool;
		}
	}

	template <>
	optional<float> extend_node_value::get_value<float>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::number_float)
		{
			return nullopt;
		}
		else
		{
			return v_bool;
		}
	}
	template <>
	optional<double> extend_node_value::get_value<double>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::number_double)
		{
			return nullopt;
		}
		else
		{
			return v_double;
		}
	}
	template <>
	optional<string_view> extend_node_value::get_value<string_view>() const
	{
		if(type_desc->_type != basic_node_type_descriptor::string)
		{
			return nullopt;
		}
		else
		{
			return v_text;
		}
	}
	template<typename T>
	optional<T> typed_cell::expect_value() const
	{
		if(!cur_typed_value)
		{
			return nullopt;
		}
		return cur_typed_value->get_value<T>();
	}
	template<typename... args>
	optional<tuple<args...>> extend_node_value::get_value() const
	{
		if(v_list.size() == 0)
		{
			return nullopt;
		}
		for(const auto i: v_list)
		{
			if(!i)
			{
				return nullopt;
			}
		}
		auto the_tuple_size = sizeof...(args);
		if(v_list.size() != the_tuple_size)
		{
			return nullopt;
		}
		return get_tuple_value_from_vector<args...>(onst vector<extend_node_value*>& v_list, std::index_sequence_for<args...>{});

	}
	template<typename... args, size_t... arg_idx>
	optional<tuple<args>...> get_tuple_value_from_vector(const vector<extend_node_value*>& v_list, std::index_sequence<arg_idx...>)
	{
		auto temp_result = make_tuple((*v_list[arg_idx]).get_value<arg>()...);

		if(!(get<arg_idx>(temp_result) &&...))
		{
			return nullopt;
		}
		return make_tuple(get<arg_idx>(temp_result).value()...);

	}
}