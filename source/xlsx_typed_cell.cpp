#include <xlsx_typed_cell.h>
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
	typed_node_type_descriptor* parse_type_from_tokens(vector<string_view> tokens)
	{
		static unordered_map<string_view, basic_value_type> str_to_type_map = {
			{"comment", basic_value_type::comment},
			{"str", basic_value_type::string},
			{"bool", basic_value_type::number_bool},
			{"uint32", basic_value_type::number_u32},
			{"int", basic_value_type::number_32},
			{"uint64", basic_value_type::number_u64},
			{"int64", basic_value_type::number_64},
			{"float", basic_value_type::number_float},
			{"double", basic_value_type::number_double},
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
				return new typed_node_type_descriptor(map_iter->second);
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
				auto result = new typed_node_type_descriptor(basic_value_type::ref_id);
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
						return new typed_node_type_descriptor(make_tuple(temp_type_result, repeat_times.value(), splitor));
					}

				}
				else
				{
					//tuple(xx, xx, seperator)
					vector<typed_node_type_descriptor*> type_vec;
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
					return new typed_node_type_descriptor(make_pair(type_vec, cur_splitor));
				}

			}
		}
		return nullptr;
	}
	
	optional<typed_node_type_descriptor::list_detail_t> typed_node_type_descriptor::get_list_detail_t() const
	{
		if(_type != basic_value_type::list)
		{
			return nullopt;
		}
		else
		{
			return std::get<typed_node_type_descriptor::list_detail_t>(_type_detail);
		}
	}

	optional<typed_node_type_descriptor::ref_detail_t> typed_node_type_descriptor::get_ref_detail_t() const
	{
		if(_type != basic_value_type::ref_id)
		{
			return nullopt;
		}
		else
		{
			return std::get<typed_node_type_descriptor::ref_detail_t>(_type_detail);
		}
	}

	optional<typed_node_type_descriptor::tuple_detail_t> typed_node_type_descriptor::get_tuple_detail_t() const
	{
		if(_type != basic_value_type::tuple)
		{
			return nullopt;
		}
		else
		{
			return std::get<typed_node_type_descriptor::tuple_detail_t>(_type_detail);
		}
	}

	typed_node_type_descriptor::typed_node_type_descriptor()
	{
		_type = basic_value_type::comment;
	}
	typed_node_type_descriptor::typed_node_type_descriptor(basic_value_type in_type)
	{
		_type = in_type;
	}
	typed_node_type_descriptor::typed_node_type_descriptor(const typed_node_type_descriptor::tuple_detail_t& tuple_detail):
		_type(basic_value_type::tuple), _type_detail(tuple_detail)
	{

	}
	typed_node_type_descriptor::typed_node_type_descriptor(const typed_node_type_descriptor::list_detail_t& list_detail):
		_type(basic_value_type::list), _type_detail(list_detail)
	{

	}
	typed_node_type_descriptor::typed_node_type_descriptor(const typed_node_type_descriptor::ref_detail_t& ref_detail)
		:_type(basic_value_type::ref_id), _type_detail(ref_detail)
	{

	}
	typed_node_type_descriptor::~typed_node_type_descriptor()
	{
		if(_type == basic_value_type::tuple)
		{
			auto temp_detail = get<typed_node_type_descriptor::tuple_detail_t>(_type_detail);
			for(auto k : temp_detail.first)
			{
				if(k)
				{
					k->~typed_node_type_descriptor();
				}
			}
		}
		else if(_type == basic_value_type::list)
		{
			auto temp_detail = get<typed_node_type_descriptor::list_detail_t>(_type_detail);
			auto k = get<0>(temp_detail);
			if(k)
			{
				k->~typed_node_type_descriptor();
			}
		}
	}
	ostream& operator<<(ostream& output_stream, const typed_node_type_descriptor& cur_type)
	{
		static unordered_map<basic_value_type, string_view> type_to_string = {
			{basic_value_type::comment, "comment"},
			{basic_value_type::string, "string"},
			{basic_value_type::number_bool, "bool"},
			{basic_value_type::number_u32, "uint32"},
			{basic_value_type::number_32, "int32"},
			{basic_value_type::number_64, "int64"},
			{basic_value_type::number_u64, "uint64"},
			{basic_value_type::number_float, "float"},
			{basic_value_type::number_double, "double"},
			{basic_value_type::list, "list"},
			{basic_value_type::ref_id, "ref"},
			{basic_value_type::tuple, "tuple"},
		};
		auto temp_iter = type_to_string.find(cur_type._type);
		if(temp_iter == type_to_string.end())
		{
			return output_stream<<"invalid";
		}
		output_stream<<temp_iter->second;
		if(cur_type._type == basic_value_type::ref_id)
		{
			auto temp_detail = std::get<typed_node_type_descriptor::ref_detail_t>(cur_type._type_detail);
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
		else if(cur_type._type == basic_value_type::tuple)
		{
			auto temp_detail = std::get<typed_node_type_descriptor::tuple_detail_t>(cur_type._type_detail);
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
		else if(cur_type._type == basic_value_type::list)
		{
			auto temp_detail = std::get<typed_node_type_descriptor::list_detail_t>(cur_type._type_detail);
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
	bool operator==(const typed_node_type_descriptor& cur, const typed_node_type_descriptor& other)
	{
		if(cur._type != other._type)
		{
			return false;
		}
		switch(cur._type)
		{
			case basic_value_type::comment:
			case basic_value_type::string:
			case basic_value_type::number_bool:
			case basic_value_type::number_32:
			case basic_value_type::number_64:
			case basic_value_type::number_double:
			case basic_value_type::number_float:
			case basic_value_type::number_u32:
			case basic_value_type::number_u64:
				return true;
			case basic_value_type::list:
			{
				if(cur._type_detail.index() != other._type_detail.index())
				{
					return false;
				}
				auto cur_detail = std::get<typed_node_type_descriptor::list_detail_t>(cur._type_detail);
				auto other_detail = std::get<typed_node_type_descriptor::list_detail_t>(other._type_detail);
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
				
			case basic_value_type::ref_id:
			{
				if(cur._type_detail.index() != other._type_detail.index())
				{
					return false;
				}
				auto cur_detail = std::get<typed_node_type_descriptor::ref_detail_t>(cur._type_detail);
				auto other_detail = std::get<typed_node_type_descriptor::ref_detail_t>(other._type_detail);
				return cur_detail == other_detail;
			}
				
			case basic_value_type::tuple:
			{
				if(cur._type_detail.index() != other._type_detail.index())
				{
					return false;
				}
				auto cur_detail = std::get<typed_node_type_descriptor::tuple_detail_t>(cur._type_detail);
				auto other_detail = std::get<typed_node_type_descriptor::tuple_detail_t>(other._type_detail);
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
	bool operator!=(const typed_node_type_descriptor& cur, const typed_node_type_descriptor& other)
	{
		return !(cur == other);
	}
	typed_node_type_descriptor* typed_value_parser::parse_type(string_view type_string)
	{
		auto all_tokens = parse_token_from_type_str(type_string);
		return parse_type_from_tokens(all_tokens);
	}
	ostream& operator<<(ostream& output_stream, const typed_value& cur_value)
	{
		switch(cur_value.type_desc->_type)
		{
			case basic_value_type::comment:
				return output_stream<<cur_value.v_text;
			case basic_value_type::number_bool:
				return output_stream<<(cur_value.v_bool?"1":"0");
			case basic_value_type::number_32:
				return output_stream<<cur_value.v_int32;
			case basic_value_type::number_u32:
				return output_stream<<cur_value.v_uint32;
			case basic_value_type::number_64:
				return output_stream<<cur_value.v_int64;
			case basic_value_type::number_u64:
				return output_stream<<cur_value.v_uint64;
			case basic_value_type::number_float:
				return output_stream<<cur_value.v_float;
			case basic_value_type::number_double:
				return output_stream<<cur_value.v_double;
			case basic_value_type::ref_id:
				return output_stream<<cur_value.v_text;
			case basic_value_type::string:
				return output_stream<<cur_value.v_text;
			case basic_value_type::list:
			{
				auto cur_list_detail = std::get<typed_node_type_descriptor::list_detail_t>(cur_value.type_desc->_type_detail);
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
			case basic_value_type::tuple:
			{
				auto cur_tuple_detail = std::get<typed_node_type_descriptor::tuple_detail_t>(cur_value.type_desc->_type_detail);
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
	
	const typed_node_type_descriptor* typed_node_type_descriptor::get_basic_type_desc(basic_value_type in_type)
	{
		static std::vector<typed_node_type_descriptor*> result = {
			new typed_node_type_descriptor(basic_value_type::comment),
			new typed_node_type_descriptor(basic_value_type::string),
			new typed_node_type_descriptor(basic_value_type::number_bool),
			new typed_node_type_descriptor(basic_value_type::number_u32),
			new typed_node_type_descriptor(basic_value_type::number_32),
			new typed_node_type_descriptor(basic_value_type::number_u64),
			new typed_node_type_descriptor(basic_value_type::number_64),
			new typed_node_type_descriptor(basic_value_type::number_float),
			new typed_node_type_descriptor(basic_value_type::number_double),
		};
		if (static_cast<uint32_t>(in_type) > static_cast<uint32_t>(basic_value_type::number_double))
		{
			return result[0];
		}
		else
		{
			return result[static_cast<uint32_t>(in_type)];
		}
	}

	typed_value::typed_value()
		:type_desc(nullptr)
	{

	}
	typed_value::typed_value(bool in_value):
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::number_bool))
	{
		v_bool = in_value;
	}
	typed_value::typed_value(uint32_t in_value) :
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::number_u32))
	{
		v_uint32 = in_value;
	}
	typed_value::typed_value(int32_t in_value) :
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::number_32))
	{
		v_int32 = in_value;
	}
	typed_value::typed_value(uint64_t in_value) :
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::number_u64))
	{
		v_uint64 = in_value;
	}
	typed_value::typed_value(int64_t in_value) :
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::number_64))
	{
		v_int64 = in_value;
	}
	typed_value::typed_value(float in_value) :
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::number_float))
	{
		v_float = in_value;
	}
	typed_value::typed_value(double in_value) :
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::number_double))
	{
		v_double = in_value;
	}
	typed_value::typed_value(string_view in_value) :
		type_desc(typed_node_type_descriptor::get_basic_type_desc(basic_value_type::string))
	{
		v_text = in_value;
	}
	typed_value::typed_value(const typed_node_type_descriptor* in_type_desc, string_view in_value):
		type_desc(in_type_desc)
	{
		v_text = in_value;
	}

	typed_value::typed_value(const typed_node_type_descriptor* in_type_desc, vector<typed_value*> in_value)
		: type_desc(in_type_desc), v_list(in_value)
	{
	}

	typed_cell::typed_cell(uint32_t in_row, uint32_t in_column, const typed_value* in_value):_row(in_row), _column(in_column), cur_typed_value(in_value)
	{

	}

	bool operator==(const typed_value& cur, const typed_value& other)
	{
		if(!cur.type_desc || ! other.type_desc)
		{
			return false;
		}
		if(*cur.type_desc!=(*other.type_desc))
		{
			// maybe one in ref 
			if (cur.type_desc->_type == basic_value_type::ref_id || other.type_desc->_type == basic_value_type::ref_id)
			{
				if (cur.type_desc->_type == basic_value_type::ref_id && other.type_desc->_type == basic_value_type::ref_id)
				{
					return false;
				}
				else
				{
					if (!cur.v_text.empty() || !other.v_text.empty())
					{
						if (cur.v_text != other.v_text)
						{
							return false;
						}
						else
						{
							return true;
						}
					}
					else
					{
						return cur.v_int32 == other.v_int32;
					}

				}
			}
			else
			{
				return false;
			}
		}
		switch(cur.type_desc->_type)
		{
		case basic_value_type::comment:
		case basic_value_type::string:
			return cur.v_text == other.v_text;
		case basic_value_type::number_double:
			return cur.v_double == other.v_double;
		case basic_value_type::number_32:
			return cur.v_int32 == other.v_int32;
		case basic_value_type::number_u32:
			return cur.v_uint32 == other.v_uint32;
		case basic_value_type::number_64:
			return cur.v_int64 == other.v_int64;
		case basic_value_type::number_u64:
			return cur.v_uint64 == other.v_uint64;
		case basic_value_type::number_bool:
			return cur.v_bool == other.v_bool;
		case basic_value_type::number_float:
			return cur.v_float == other.v_float;
		case basic_value_type::list:
		case basic_value_type::tuple:
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
		case basic_value_type::ref_id:
		{
			auto cur_ref_detail_opt = cur.type_desc->get_ref_detail_t();
			if (!cur_ref_detail_opt)
			{
				return 0;
			}
			auto cur_ref_detail = cur_ref_detail_opt.value();
			if (get<2>(cur_ref_detail) == "str")
			{
				return cur.v_text == other.v_text;
			}
			else
			{
				return cur.v_int32 == other.v_int32;
			}
		}
			
		default:
			return false;
		}
	}
	bool operator!=(const typed_value& cur, const typed_value& other)
	{
		return !(cur == other);
	}

	size_t typed_value_hash::operator()(const typed_value* s) const
	{
		if (!s)
		{
			return 0;
		}
		switch(s->type_desc->_type)
		{
		case basic_value_type::comment:
		case basic_value_type::string:
			return std::hash<std::string_view>()(s->v_text);
		case basic_value_type::number_double:
			return std::hash<double>()(s->v_double);
		case basic_value_type::number_32:
			return std::hash<std::int32_t>()(s->v_int32);
		case basic_value_type::number_u32:
			return std::hash<std::uint32_t>()(s->v_uint32);
		case basic_value_type::number_64:
			return std::hash<std::int64_t>()(s->v_int64);
		case basic_value_type::number_u64:
			return std::hash<std::uint64_t>()(s->v_uint64);
		case basic_value_type::number_bool:
			return std::hash<bool>()(s->v_bool);
		case basic_value_type::number_float:
			return std::hash<float>()(s->v_float);
		case basic_value_type::list:
		case basic_value_type::tuple:
		{
			auto cur_size = s->v_list.size();
			std::size_t result_hash = 0;
			for (const auto & i : s->v_list)
			{
				result_hash += operator()(i) / cur_size;
			}
			return result_hash;
		}
		case basic_value_type::ref_id:
		{
			auto cur_ref_detail_opt = s->type_desc->get_ref_detail_t();
			if (!cur_ref_detail_opt)
			{
				return 0;
			}
			auto cur_ref_detail = cur_ref_detail_opt.value();
			if (get<2>(cur_ref_detail) == "str")
			{
				return std::hash<std::string_view>()(s->v_text);
			}
			else
			{
				return std::hash<std::int32_t>()(s->v_int32);
			}
		}
			
		default:
			return 0;
		}
	}
	bool typed_value_ptr_equal::operator()(const typed_value* from, const typed_value* to) const
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

	typed_value::~typed_value()
	{
		
	}
	void typed_value::cleanup_resource()
	{
		for (auto i : v_list)
		{
			if (i)
			{
				i->cleanup_resource();
				delete i;
			}
		}
		v_list.clear();
		type_desc = nullptr;
	}
	
	template <>
	optional<std::uint32_t> typed_value::expect_simple_value<uint32_t>() const
	{
		auto cur_type = type_desc->_type;
		if(cur_type == basic_value_type::number_u32)
		{
			return v_uint32;
		}
		else if(cur_type == basic_value_type::number_32)
		{
			if (v_int32 < 0)
			{
				return nullopt;
			}
			return static_cast<std::uint32_t>(v_int32);
		}
		else
		{
			return nullopt;
		}
	}

	template <>
	optional<std::int32_t> typed_value::expect_simple_value<int32_t>() const
	{
		auto cur_type = type_desc->_type;
		if(cur_type == basic_value_type::number_32)
		{
			return v_int32;
		}
		else if(cur_type == basic_value_type::number_u32)
		{
			if (v_uint32 > numeric_limits<uint32_t>::max() / 2)
			{
				return nullopt;
			}
			else
			{
				return static_cast<int>(v_uint32);
			}
			
		}
		else
		{
			return nullopt;
		}
	}

	template <>
	optional<std::int64_t> typed_value::expect_simple_value<int64_t>() const
	{
		if(type_desc->_type != basic_value_type::number_64)
		{
			return nullopt;
		}
		else
		{
			return v_int64;
		}
	}
	template <>
	optional<std::uint64_t> typed_value::expect_simple_value<uint64_t>() const
	{
		if(type_desc->_type != basic_value_type::number_u64)
		{
			return nullopt;
		}
		else
		{
			return v_uint64;
		}
	}
	template <>
	optional<bool> typed_value::expect_simple_value<bool>() const
	{
		if(type_desc->_type != basic_value_type::number_bool)
		{
			return nullopt;
		}
		else
		{
			return v_bool;
		}
	}

	template <>
	optional<float> typed_value::expect_simple_value<float>() const
	{
		if(type_desc->_type != basic_value_type::number_float)
		{
			return nullopt;
		}
		else
		{
			return v_float;
		}
	}
	template <>
	optional<double> typed_value::expect_simple_value<double>() const
	{
		if(type_desc->_type != basic_value_type::number_double)
		{
			return nullopt;
		}
		else
		{
			return v_double;
		}
	}
	template <>
	optional<string_view> typed_value::expect_simple_value<string_view>() const
	{

		if(v_text.empty())
		{
			return nullopt;
		}
		else
		{
			return v_text;
		}
	}
	uint32_t typed_value::memory_details() const
	{
		uint32_t result = 0;
		result += sizeof(typed_value);
		for (const auto& i : v_list)
		{
			result += i->memory_details();
		}
		result += v_list.capacity() * 4 + sizeof(vector<typed_value*>);
		return result;
	}
	uint32_t typed_cell::memory_details() const
	{
		uint32_t result = sizeof(typed_cell);
		if (cur_typed_value)
		{
			result += cur_typed_value->memory_details();
		}
		return result;
	}
	typed_value* typed_value_parser::parse_value_with_type(const typed_node_type_descriptor* node_type, string_view text)
	{
		typed_value* temp_result = new typed_value;
		if (parse_value_with_address(node_type, text, temp_result))
		{
			return temp_result;
		}
		else
		{
			delete temp_result;
			return nullptr;
		}
	}
	bool typed_value_parser::parse_value_with_type(const typed_node_type_descriptor* node_type, string_view text, typed_value& result)
	{
		return parse_value_with_address(node_type, text, &result);
	}

	bool typed_value_parser::parse_value_with_address(const typed_node_type_descriptor* node_type, string_view text, typed_value* result)
	{
		text.remove_prefix(min(text.find_first_not_of(" "), text.size()));
		text.remove_suffix(text.size() - min(text.find_last_not_of(" ") + 1, text.size()));
		if (text.size() == 0)
		{
			return false;
		}
		auto current_double_value = cast_numeric(text);
		switch (node_type->_type)
		{
		case basic_value_type::comment:
			return false;
		case basic_value_type::string:
			new(result) typed_value(text);
			return true;
		case basic_value_type::number_bool:
			if (text == "1"sv)
			{
				new(result) typed_value(true);
				return true;
			}
			else if (text == "0"sv)
			{
				new(result) typed_value(false);
				return true;
			}
			else
			{
				return false;
			}
		case basic_value_type::number_32:
			if (current_double_value.has_value())
			{
				new(result) typed_value(static_cast<int32_t>(current_double_value.value()));
				return true;
			}
			else
			{
				return false;
			}
		case basic_value_type::number_u32:
			if (current_double_value.has_value())
			{
				new(result) typed_value(static_cast<uint32_t>(current_double_value.value()));
				return true;
			}
			else
			{
				return false;
			}
		case basic_value_type::number_64:
			if (current_double_value.has_value())
			{
				new(result) typed_value(static_cast<int64_t>(current_double_value.value()));
				return true;
			}
			else
			{
				return false;
			}
		case basic_value_type::number_u64:
			if (current_double_value.has_value())
			{
				return new(result) typed_value(static_cast<uint64_t>(current_double_value.value()));
			}
			else
			{
				return false;
			}
		case basic_value_type::number_float:
			if (current_double_value.has_value())
			{
				new(result) typed_value(static_cast<float>(current_double_value.value()));
				return true;
			}
			else
			{
				return false;
			}
		case basic_value_type::number_double:
			if (current_double_value.has_value())
			{
				new(result) typed_value(current_double_value.value());
				return true;
			}
			else
			{
				return false;
			}
		case basic_value_type::ref_id:
		{
			auto cur_ref_detail = std::get<typed_node_type_descriptor::ref_detail_t>(node_type->_type_detail);
			if (std::get<2>(cur_ref_detail) == "str"sv)
			{
				new(result) typed_value(node_type, text);
				return true;
			}
			else
			{
				new(result) typed_value(static_cast<uint64_t>(current_double_value.value()));
				return true;
			}
		}
		case basic_value_type::tuple:
		{
			auto cur_tuple_detail = std::get<typed_node_type_descriptor::tuple_detail_t>(node_type->_type_detail);
			char sep = cur_tuple_detail.second;
			auto type_list = cur_tuple_detail.first;
			text = strip_parenthesis(text);
			auto tokens = split_string(text, sep);
			if (tokens.size() != type_list.size())
			{
				return false;
			}
			vector<typed_value *> sub_values;
			for (int i = 0; i < type_list.size(); i++)
			{
				typed_value* temp_result = new typed_value;
				temp_result->~typed_value();
				auto cur_value = parse_value_with_type(type_list[i], tokens[i], *temp_result);
				if (!cur_value)
				{
					for (auto& one_value : sub_values)
					{
						delete one_value;
					}
					delete temp_result;
					return false;
				}
				sub_values.push_back(temp_result);
			}
			new(result) typed_value(node_type, sub_values);
			return true;
		}
		case basic_value_type::list:
		{
			auto cur_list_detail = std::get<typed_node_type_descriptor::list_detail_t>(node_type->_type_detail);
			uint32_t list_size = std::get<uint32_t>(cur_list_detail);
			char sep = std::get<char>(cur_list_detail);
			auto unit_type = std::get<typed_node_type_descriptor *>(cur_list_detail);
			text = strip_parenthesis(text);
			auto tokens = split_string(text, sep);
			vector<typed_value *> sub_values;
			if (list_size == 0)
			{
				for (auto one_token : tokens)
				{
					typed_value* temp_result = new typed_value;
					temp_result->~typed_value();
					auto cur_value = parse_value_with_type(unit_type, one_token, *temp_result);
					if (!cur_value)
					{
						for (auto& one_value : sub_values)
						{
							delete one_value;
						}
						delete temp_result;
						return false;
					}
					sub_values.push_back(temp_result);
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
					typed_value* temp_result = new typed_value;
					temp_result->~typed_value();
					auto cur_value = parse_value_with_type(unit_type, one_token, *temp_result);
					if (!cur_value)
					{
						for (auto& one_value : sub_values)
						{
							delete one_value;
						}
						delete temp_result;
						return false;
					}
					sub_values.push_back(temp_result);
				}
			}
			new(result) typed_value(node_type, sub_values);
			return true;
		}
		default:
			return false;
		}
	}
	const typed_value* typed_value_parser::match_node(string_view text)
	{
		return typed_value_parser::parse_value_with_type(type_desc, text);
	}
	bool typed_value_parser::match_node(string_view text, typed_value& result)
	{
		return typed_value_parser::parse_value_with_type(type_desc, text, result);
	}

}