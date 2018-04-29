#pragma once

#include <xlsx_types.h>
#include <cstdint>
namespace xlsx_reader {
    struct cell_value
    {
        cell_value();
        cell_type _type;
        std::string_view _text;
        union
        {
            std::uint32_t int_u32_v;
            std::int32_t int_32_v;
            std::uint64_t int_u64_v;
            std::int64_t int_64_v;
            float float_v;
            double double_v;
            bool bool_v;
        };
        std::uint32_t _row;
        std::uint64_t _column;

    };
    class cell: public cell_value
    {
    public:
        cell(cell&) = default;
        template <typename T>
        T get_value() const;
        void set_value(bool _value);
        void set_value(std::int32_t _value);
        void set_value(std::uint32_t _value);
        void set_value(std::int64_t _value);
        void set_value(std::uint64_t _value);
        void set_value(float _value);
        void set_value(double _value);
        void set_value(const date& _value);
        void set_value(const time& _value);
        void set_value(const datetime& _value);
        void set_value(std::string_view _value);
        void from_formula(std::string_view _value);
        void from_error(std::string_view _value);
        void infer_value(std::string_view _value);
        cell_type get_type() const;

        std::uint32_t get_column() const;
        std::uint32_t get_row() const;
        std::pair<std::uint32_t, std::uint32_t> get_row_column() const;
        std::string to_string() const;
        cell(uint32_t row_idx, uint32_t col_idx);
        cell &operator=(const cell &rhs);
        bool operator==(const cell &comparand) const;

    };
    template<>
    bool cell::get_value<bool>() const;
    template<>
    std::int32_t cell::get_value<std::int32_t>() const;
    template<>
    std::uint32_t cell::get_value<std::uint32_t>() const;
    template<>
    std::int64_t cell::get_value<std::int64_t>() const;
    template<>
    std::uint64_t cell::get_value<std::uint64_t>() const;
    template<>
    float cell::get_value<float>() const;
    template<>
    double cell::get_value<double>() const;
    template<>
    date cell::get_value<date>() const;
    template<>
    time cell::get_value<time>() const;
    template<>
    datetime cell::get_value<datetime>() const;
    template<>
    std::string_view cell::get_value<std::string_view>() const;

}