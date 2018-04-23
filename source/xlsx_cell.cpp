#include <xlsx_cell.h>
#include <xlsx_types.h>
namespace xlsx_reader
{
    using namespace std;
    void cell::set_value(bool _value)
    {
        _type = cell_type::number_bool;
        bool_v = _value;
    }
    void cell::set_value(uint32_t _value)
    {
        _type = cell_type::number_u32;
        int_u32_v = _value;
    }
    void cell::set_value(int32_t _value)
    {
        _type = cell_type::number_32;
        int_32_v = _value;
    }
    void cell::set_value(uint64_t _value)
    {
        _type = cell_type::number_u64;
        int_u64_v = _value;
    }
    void cell::set_value(int64_t _value)
    {
        _type = cell_type::number_64;
        int_64_v = _value;
    }
    void cell::set_value(float _value)
    {
        _type = cell_type::number_float;
        float_v = _value;
    }
    void cell::set_value(double _value)
    {
        _type = cell_type::number_double;
        double_v = _value;
    }
    void cell::set_value(string_view _value)
    {
        _type = cell_type::inline_string;
        _text = _value;
    }
    void cell::set_value(const date& _value)
    {
        _type = cell_type::date;
        double_v = data.to_number(base_data());
    }
    void cell::set_value(const datetime& _value)
    {
        _type = cell_type::datetime;
        double_v = datetime.to_number();
    }
    void cell::set_value(const time& _value)
    {
        _type = cell_type::time;
        double_v = time.to_number();
    }
    uint32_t get_row() const
    {
        return row;
    }
    uint32_t get_column() const
    {
        return column;
    }
    pair<uint32_t, uint32_t> get_row_column() const
    {
        return make_pair(row, column);
    }
    cell_type get_type() const
    {
        return _type;
    }
    bool cell::operator==(const cell& rhs) const
    {
        if(_type != rhs._type or _text != rhs._text or double_v != rhs.double_v)
        {
            return false;
        }
        else
        {
            return true;
        }

    }
    cell& cell:operator=(const cell& rhs)
    {
        _type = rhs._type;
        _text = rhs._text;
        _row = rhs._row;
        _column = rhs._column;
        double_v = rhs.double_v;
    }

    bool cell::get_value<bool>() const
    {
        return _bool_v;
    }
    uint32_t cell::get_value<uint32_t>() const
    {
        return int_u32_v;
    }
    int32_t cell::get_value<int32_t>() const
    {
        return int_32_v;
    }
    uint64_t cell::get_value<uint64_t>() const
    {
        return int_u64_v;
    }
    int64_t cell::get_value<int64_t>() const
    {
        return int_64_v;
    }
    float cell::get_value<float>() const
    {
        return float_v;
    }
    double cell::get_value<double>() const
    {
        return double_v;
    }
    string_view cell::get_value<string_view>() const
    {
        return _text;
    }

    string cell::to_string() const
    {
        switch(_type)
        {
            case cell_type::empty:
                return ""
            case cell_type::date
            return ""

        }
        return ""
    }


}
