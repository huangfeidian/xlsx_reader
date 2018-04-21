#include <xlsx_types>

namepsace xlsx_reader{
    class cell
    {
        static const std::unordered_map<std::string, int> error_codes = {
        {"#NULL!", 0},
        {"#DIV/0!", 1},
        {"#VALUE!", 2},
        {"#REF!", 3},
        {"#NAME?", 4},
        {"#NUM!", 5},
        {"#N/A!", 6}};
    }
}