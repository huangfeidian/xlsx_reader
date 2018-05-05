#include <xlsx_types.h>
#include <optional>
#include <ctime>
#include <cmath>
#include <vector>

namespace xlsx_reader
{
using namespace std;
optional<double> cast_numeric(string_view s)
{
    double result = 0;
	char* result_end = static_cast<char*>(nullptr);
	string current_str(s);
	auto cast_result = strtod(&(*current_str.cbegin()), &result_end);
	if(result_end != &(*current_str.cend()))
    {
		return {};
    }
    else
    {
        return result;
    }
}
optional<int> cast_int(string_view s)
{
    int result = 0;
	char* result_end = static_cast<char*>(nullptr);
	string current_str(s);
	auto cast_result = strtol(&(*current_str.cbegin()), &result_end, 10);
	if(result_end != &(*current_str.cend()))
    {
		return {};
    }
    else
    {
        return result;
    }
}

optional<double> cast_percentage(string_view s)
{
    if (s.back() == '%s')
    {
        return cast_numeric(string_view(s.data(), s.size() - 1));
    }
    else
    {
        return {};
    }
}

optional<time> cast_time(string_view s)
{
    time result;
    std::vector<string_view> time_components;
    std::size_t prev = 0;
    auto colon_index = s.find(':');

    while (colon_index != std::string::npos)
    {
        time_components.push_back(s.substr(prev, colon_index - prev));
        prev = colon_index + 1;
        colon_index = s.find(':', colon_index + 1);
    }

    time_components.push_back(s.substr(prev, colon_index - prev));

    if (time_components.size() < 2 || time_components.size() > 3)
    {
        return {};
    }

    std::vector<double> numeric_components;

    for (auto component : time_components)
    {
        if (component.empty() || (string_view(component.data(), component.find('.')).size() > 2))
        {
            return {};
        }

        for (auto d : component)
        {
            if (!(d >= '0' && d <= '9') && d != '.')
            {
                return {};
            }
        }

        auto without_leading_zero = component.front() == '0' ? string_view(component.data() + 1, component.size() - 1) : component;
        char *numeric_end = static_cast<char *>(nullptr);
        auto numeric = std::strtod(without_leading_zero.data(), &numeric_end);

        numeric_components.push_back(numeric);
    }

    result.hour = static_cast<int>(numeric_components[0]);
    result.minute = static_cast<int>(numeric_components[1]);

    if (std::fabs(static_cast<double>(result.minute) - numeric_components[1]) > std::numeric_limits<double>::epsilon())
    {
        result.minute = result.hour;
        result.hour = 0;
        result.second = static_cast<int>(numeric_components[1]);
        result.microsecond = static_cast<int>((numeric_components[1] - result.second) * 1E6);
    }
    else if (numeric_components.size() > 2)
    {
        result.second = static_cast<int>(numeric_components[2]);
        result.microsecond = static_cast<int>((numeric_components[2] - result.second) * 1E6);
    }

    return result;
}

std::tm safe_localtime(std::time_t raw_time)
{
#ifdef _MSC_VER
    std::tm result;
    localtime_s(&result, &raw_time);

    return result;
#else
    return *localtime(&raw_time);
#endif
}
int svtoi(string_view s)
{
	return stoi(string(s));
}

const string& error_to_string(cell_error in_error)
{
    const static vector<string> error_strings = {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A!"};
    return error_strings[static_cast<int>(in_error)];

}
cell_error error_from_string(const std::string& in_string)
{
    const static std::unordered_map<std::string, cell_error> cell_errorcodes = unordered_map<string, cell_error>{
    {"#NULL!", cell_error::E_NULL},
    {"#DIV/0!", cell_error::E_DIV_0},
    {"#VALUE!", cell_error::E_VALUE},
    {"#REF!", cell_error::E_REF},
    {"#NAME?", cell_error::E_NAME},
    {"#NUM!", cell_error::E_NUM},
    {"#N/A!", cell_error::E_NA}};
    auto cur_iter = cell_errorcodes.find(in_string);
    if(cur_iter == cell_errorcodes.end())
    {
        return cell_error::E_NULL;
    }
    else
    {
        return cur_iter->second;
    }
}

time time::from_number(double raw_time)
{
    time result;

    double integer_part;
    double fractional_part = std::modf(static_cast<double>(raw_time), &integer_part);

    fractional_part *= 24;
    result.hour = static_cast<int>(fractional_part);
    fractional_part = 60 * (fractional_part - result.hour);
    result.minute = static_cast<int>(fractional_part);
    fractional_part = 60 * (fractional_part - result.minute);
    result.second = static_cast<int>(fractional_part);
    fractional_part = 1000000 * (fractional_part - result.second);
    result.microsecond = static_cast<int>(fractional_part);

    if (result.microsecond == 999999 && fractional_part - result.microsecond > 0.5)
    {
        result.microsecond = 0;
        result.second += 1;

        if (result.second == 60)
        {
            result.second = 0;
            result.minute += 1;

            // TODO: too much nesting
            if (result.minute == 60)
            {
                result.minute = 0;
                result.hour += 1;
            }
        }
    }

    return result;
}

time::time(int hour_, int minute_, int second_, int microsecond_)
    : hour(hour_), minute(minute_), second(second_), microsecond(microsecond_)
{
}

bool time::operator==(const time &comparand) const
{
    return hour == comparand.hour && minute == comparand.minute && second == comparand.second && microsecond == comparand.microsecond;
}

time::time(string_view time_string)
    : hour(0), minute(0), second(0), microsecond(0)
{
    std::string_view remaining = time_string;
    auto colon_index = remaining.find(':');
    hour = svtoi(remaining.substr(0, colon_index));
    remaining = remaining.substr(colon_index + 1);
    colon_index = remaining.find(':');
    minute = svtoi(remaining.substr(0, colon_index));
    colon_index = remaining.find(':');

    if (colon_index != std::string::npos)
    {
        remaining = remaining.substr(colon_index + 1);
        second = svtoi(remaining);
    }
}

double time::to_number() const
{
    std::uint64_t microseconds = static_cast<std::uint64_t>(microsecond);
    microseconds += static_cast<std::uint64_t>(second * 1e6);
    microseconds += static_cast<std::uint64_t>(minute * 1e6 * 60);
    auto microseconds_per_hour = static_cast<std::uint64_t>(1e6) * 60 * 60;
    microseconds += static_cast<std::uint64_t>(hour) * microseconds_per_hour;
    auto number = microseconds / (24.0 * microseconds_per_hour);
    auto hundred_billion = static_cast<std::uint64_t>(1e9) * 100;
    number = std::floor(number * hundred_billion + 0.5) / hundred_billion;

    return number;
}
string time::to_string() const
{
	return std::to_string(hour)
		+ ":" + std::to_string(minute) + ":" + std::to_string(second) + ":" + std::to_string(microsecond);
}
time time::now()
{
    std::tm now = safe_localtime(std::time(nullptr));
    return time(now.tm_hour, now.tm_min, now.tm_sec);
}

date::date(int year_, int month_, int day_)
    : year(year_), month(month_), day(day_)
{
}

date date::from_number(int days_since_base_year, calendar base_date)
{
    date result(0, 0, 0);

    if (base_date == calendar::mac_1904)
    {
        days_since_base_year += 1462;
    }

    if (days_since_base_year == 60)
    {
        result.day = 29;
        result.month = 2;
        result.year = 1900;

        return result;
    }
    else if (days_since_base_year < 60)
    {
        days_since_base_year++;
    }

    int l = days_since_base_year + 68569 + 2415019;
    int n = int((4 * l) / 146097);
    l = l - int((146097 * n + 3) / 4);
    int i = int((4000 * (l + 1)) / 1461001);
    l = l - int((1461 * i) / 4) + 31;
    int j = int((80 * l) / 2447);
    result.day = l - int((2447 * j) / 80);
    l = int(j / 11);
    result.month = j + 2 - (12 * l);
    result.year = 100 * (n - 49) + i + l;

    return result;
}

bool date::operator==(const date &comparand) const
{
    return year == comparand.year && month == comparand.month && day == comparand.day;
}

bool date::operator!=(const date &comparand) const
{
    return !(*this == comparand);
}

int date::to_number(calendar base_date) const
{
    if (day == 29 && month == 2 && year == 1900)
    {
        return 60;
    }

    int days_since_1900 = int((1461 * (year + 4800 + int((month - 14) / 12))) / 4) + int((367 * (month - 2 - 12 * ((month - 14) / 12))) / 12) - int((3 * (int((year + 4900 + int((month - 14) / 12)) / 100))) / 4) + day - 2415019 - 32075;

    if (days_since_1900 <= 60)
    {
        days_since_1900--;
    }

    if (base_date == calendar::mac_1904)
    {
        return days_since_1900 - 1462;
    }

    return days_since_1900;
}

string date::to_string() const
{
	return std::to_string(year) + "/" + std::to_string(month) + "/" + std::to_string(day);
}
date date::today()
{
    std::tm now = safe_localtime(std::time(nullptr));
    return date(1900 + now.tm_year, now.tm_mon + 1, now.tm_mday);
}

int date::weekday() const
{
    auto year_temp = (month == 1 || month == 2) ? year - 1 : year;
    auto month_temp = month == 1 ? 13 : month == 2 ? 14 : month;
    auto day_temp = day + 1;

    auto days = day_temp + static_cast<int>(13 * (month_temp + 1) / 5.0) + (year_temp % 100) + static_cast<int>((year_temp % 100) / 4.0);
    auto gregorian = days + static_cast<int>(year_temp / 400.0) - 2 * year_temp / 100;
    auto julian = days + 5 - year_temp / 100;

    int weekday = (year_temp > 1582 ? gregorian : julian) % 7;

    return weekday == 0 ? 7 : weekday;
}
datetime datetime::from_number(double raw_time, calendar base_date)
{
    auto date_part = date::from_number(static_cast<int>(raw_time), base_date);
    auto time_part = time::from_number(raw_time);

    return datetime(date_part.year, date_part.month, date_part.day, time_part.hour, time_part.minute, time_part.second,
        time_part.microsecond);
}

bool datetime::operator==(const datetime &comparand) const
{
    return year == comparand.year 
        && month == comparand.month
        && day == comparand.day
        && hour == comparand.hour
        && minute == comparand.minute
        && second == comparand.second
        && microsecond == comparand.microsecond;
}

double datetime::to_number(calendar base_date) const
{
    return date(year, month, day).to_number(base_date)
        + time(hour, minute, second, microsecond).to_number();
}

string datetime::to_string() const
{
    return std::to_string(year) + "/" + std::to_string(month) + "/" + std::to_string(day) + " " + std::to_string(hour)
        + ":" + std::to_string(minute) + ":" + std::to_string(second) + ":" + std::to_string(microsecond);
}

datetime datetime::now()
{
    return datetime(date::today(), time::now());
}

datetime datetime::today()
{
    return datetime(date::today(), time(0, 0, 0, 0));
}

datetime::datetime(int year_, int month_, int day_, int hour_, int minute_, int second_, int microsecond_)
    : year(year_), month(month_), day(day_), hour(hour_), minute(minute_), second(second_), microsecond(microsecond_)
{
}

datetime::datetime(const date &d, const time &t)
    : year(d.year),
      month(d.month),
      day(d.day),
      hour(t.hour),
      minute(t.minute),
      second(t.second),
      microsecond(t.microsecond)
{
}

int datetime::weekday() const
{
    return date(year, month, day).weekday();
}

datetime datetime::from_iso_string(string_view string)
{
    datetime result(1900, 1, 1);

    auto separator_index = string.find('-');
    result.year = svtoi(string.substr(0, separator_index));
    result.month = svtoi(string.substr(separator_index + 1, string.find('-', separator_index + 1)));
    separator_index = string.find('-', separator_index + 1);
    result.day = svtoi(string.substr(separator_index + 1, string.find('T', separator_index + 1)));
    separator_index = string.find('T', separator_index + 1);
    result.hour = svtoi(string.substr(separator_index + 1, string.find(':', separator_index + 1)));
    separator_index = string.find(':', separator_index + 1);
    result.minute = svtoi(string.substr(separator_index + 1, string.find(':', separator_index + 1)));
    separator_index = string.find(':', separator_index + 1);
    result.second = svtoi(string.substr(separator_index + 1, string.find('Z', separator_index + 1)));

    return result;
}

std::string datetime::to_iso_string() const
{
    auto fill_zero = [](const string& input_string, int length = 2)
    {
        if (input_string.size() >= length)
        {
            return input_string;
        }

        return std::string(length - input_string.size(), '0') + input_string;
	};
    return std::to_string(year) + "-" + fill_zero(std::to_string(month)) + "-" + fill_zero(std::to_string(day)) + "T"
        + fill_zero(std::to_string(hour)) + ":" + fill_zero(std::to_string(minute)) + ":" + fill_zero(std::to_string(second)) + "Z";
}

}