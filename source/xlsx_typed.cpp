#include <xlsx_typed.h>
#include <iostream>
namespace {
    using namespace xlsx_reader;
    using namespace std;

}
namespace xlsx_reader{
    using namespace std;
    typed_header::typed_header(const extend_node_type_descriptor* in_type_desc, string_view in_header_name, string_view in_header_comment):type_desc(in_type_desc), header_name(in_header_name), header_comment(in_header_comment)
    {

    }
    std::ostream& operator<<(std::ostream& output_stream, const typed_header& in_typed_header)
    {
        output_stream<<"name: "<< in_typed_header.header_name<<" type_desc: "<<*in_typed_header.type_desc<<" comment:"<<in_typed_header.header_comment<<endl;
        return output_stream;
    }
    void typed_worksheet::load_worksheet_from_string(string_view input_string)
    {
        worksheet::load_worksheet_from_string(input_string);
        convert_cell_to_typed_value();
    }
    bool typed_worksheet::convert_typed_header()
    {
        int column_idx = 0;
        auto header_name_row = get_row(0);
        for(const auto& i: header_name_row)
        {
            if(i.first != column_idx)
            {
                cout<<"not continuous header row, current is "<< i.first<<" while expecting "<< column_idx<<endl;
                return false;
            }
            
            auto cur_cell_value = i.second;
            if(!cur_cell_value)
            {
                cerr<<"empty header name at idx "<<i.first<<endl;
                return false;
            }
            if(cur_cell_value->_type != cell_type::shared_string && cur_cell_value->_type != cell_type::inline_string)
            {
                cerr<<"invalid value "<<*cur_cell_value<<" for header name at column "<<i.first<<endl;
                return false;
            }
            auto cur_header_name = cur_cell_value->get_value<string_view>();
            cur_cell_value = get_cell(1, column_idx);
            if(!cur_cell_value)
            {
                cerr<<"empty cell type value for header "<< cur_header_name<<endl;
                return false;
            }
            if(cur_cell_value->_type != cell_type::shared_string && cur_cell_value->_type != cell_type::inline_string)
            {
                cerr<<"invalid value "<<*cur_cell_value<<" for header type at column "<<i.first<<endl;
                return false;
            }
            auto cur_type_desc = extend_node_value_constructor::parse_type(cur_cell_value->get_value<string_view>());
            if(!cur_type_desc)
            {
                cerr<<"invalid type desc "<<cur_cell_value->get_value<string_view>()<<"for header type at column "<<i.first<<endl;
                return false;
            }
            string_view header_comment;
            cur_cell_value = get_cell(2, column_idx);
            if(cur_cell_value)
            {
                header_comment = cur_cell_value->get_value<string_view>();
            }
            typed_headers.emplace_back(cur_header_name, *cur_type_desc, header_comment);
            column_idx += 1;
        }
    }
    void typed_worksheet::convert_cell_to_typed_value()
    {
        for(const auto i: row_info)
        {
            if(i.first <= 2)
            {
                continue;
            }
            map<std::uint32_t, const typed_cell*> cur_row_typed_info;
            for(const auto& j: i.second)
            {
                if(!j.second)
                {
                    continue;
                }
                auto cur_typed_cell = extend_node_value_constructor::parse_node(typed_headers[j.first].type_desc, j.second);
                if(!cur_typed_cell)
                {
                    continue;
                }
                cur_row_typed_info[j.first] = cur_typed_cell;
            }
            typed_row_info[i.first] = cur_row_typed_info;
        }
    }

}