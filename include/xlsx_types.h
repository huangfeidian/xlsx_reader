#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <string_view>
#include "xlsx_types_forward.h"
#include <optional>
#include <string>
namespace xlsx_reader
{


	using relation_desc = std::tuple<std::string, std::string, std::string>;
	using sheet_desc = std::tuple<std::string, std::uint32_t, std::string>;
	enum class cell_type
	{
		empty,
		date,
		time,
		datetime,
		inline_string,
		number_bool,
		number_u32,
		number_32,
		number_u64,
		number_64,
		number_float,
		number_double,
		shared_string,
		formula_string,
		error
	};
	enum class cell_error: std::uint32_t
	{
		E_NULL,
		E_DIV_0,
		E_VALUE,
		E_REF,
		E_NAME,
		E_NUM,
		E_NA,
	};
	enum class calendar
	{
		windows_1900,
		mac_1904,
	};

	const std::string& error_to_string(cell_error in_error);
	cell_error error_from_string(const std::string& in_string);



	struct time
	{
		/// <summary>
		/// Return the current time according to the system time.
		/// </summary>
		static time now();

		/// <summary>
		/// Return a time from a number representing a fraction of a day.
		/// The integer part of number will be ignored.
		/// 0.5 would return time(12, 0, 0, 0) or noon, halfway through the day.
		/// </summary>
		static time from_number(double number);

		/// <summary>
		/// Constructs a time object from an optional hour, minute, second, and microsecond.
		/// </summary>
		explicit time(int hour_ = 0, int minute_ = 0, int second_ = 0, int microsecond_ = 0);

		/// <summary>
		/// Constructs a time object from a string representing the time.
		/// </summary>
		explicit time(std::string_view time_string);

		/// <summary>
		/// Returns a numeric representation of the time in the range 0-1 where the value
		/// is equal to the fraction of the day elapsed.
		/// </summary>
		double to_number() const;

		/// <summary>
		/// Returns true if this time is equivalent to comparand.
		/// </summary>
		bool operator==(const time &comparand) const;

		/// <summary>
		/// The hour
		/// </summary>
		int hour;

		/// <summary>
		/// The minute
		/// </summary>
		int minute;

		/// <summary>
		/// The second
		/// </summary>
		int second;

		/// <summary>
		/// The microsecond
		/// </summary>
		int microsecond;
		std::string to_string() const;
	};

	struct date
	{
		/// <summary>
		/// Return the current date according to the system time.
		/// </summary>
		static date today();

		/// <summary>
		/// Return a date by adding days_since_base_year to base_date.
		/// This includes leap years.
		/// </summary>
		static date from_number(int days_since_base_year, calendar base_date);

		/// <summary>
		/// Constructs a data from a given year, month, and day.
		/// </summary>
		date(int year_ = 0, int month_ = 0, int day_ = 0);

		/// <summary>
		/// Return the number of days between this date and base_date.
		/// </summary>
		int to_number(calendar base_date) const;

		/// <summary>
		/// Calculates and returns the day of the week that this date represents in the range
		/// 0 to 6 where 0 represents Sunday.
		/// </summary>
		int weekday() const;

		/// <summary>
		/// Return true if this date is equal to comparand.
		/// </summary>
		bool operator==(const date &comparand) const;

		/// <summary>
		/// Return true if this date is equal to comparand.
		/// </summary>
		bool operator!=(const date &comparand) const;

		/// <summary>
		/// The year
		/// </summary>
		int year;

		/// <summary>
		/// The month
		/// </summary>
		int month;

		/// <summary>
		/// The day
		/// </summary>
		int day;
		std::string to_string() const;
	};

	struct datetime
	{
		static datetime now();

		/// <summary>
		/// Returns the current date and time according to the system time.
		/// This is equivalent to datetime::now().
		/// </summary>
		static datetime today();

		/// <summary>
		/// Returns a datetime from number by converting the integer part into
		/// a date and the fractional part into a time according to date::from_number
		/// and time::from_number.
		/// </summary>
		static datetime from_number(double number, calendar base_date);

		/// <summary>
		/// Returns a datetime equivalent to the ISO-formatted string iso_string.
		/// </summary>
		static datetime from_iso_string(std::string_view iso_string);

		/// <summary>
		/// Constructs a datetime from a date and a time.
		/// </summary>
		datetime(const date &d, const time &t);

		/// <summary>
		/// Constructs a datetime from a year, month, and day plus optional hour, minute, second, and microsecond.
		/// </summary>
		datetime(int year_ = 0, int month_ = 0, int day_ = 0, int hour_ = 0, int minute_ = 0, int second_ = 0, int microsecond_ = 0);

		/// <summary>
		/// Returns a string represenation of this date and time.
		/// </summary>
		std::string to_string() const;

		/// <summary>
		/// Returns an ISO-formatted string representation of this date and time.
		/// </summary>
		std::string to_iso_string() const;

		/// <summary>
		/// Returns this datetime as a number of seconds since 1900 or 1904 (depending on base_date provided).
		/// </summary>
		double to_number(calendar base_date) const;

		/// <summary>
		/// Returns true if this datetime is equivalent to comparand.
		/// </summary>
		bool operator==(const datetime &comparand) const;

		/// <summary>
		/// Calculates and returns the day of the week that this date represents in the range
		/// 0 to 6 where 0 represents Sunday.
		/// </summary>
		int weekday() const;

		/// <summary>
		/// The year
		/// </summary>
		int year;

		/// <summary>
		/// The month
		/// </summary>
		int month;

		/// <summary>
		/// The day
		/// </summary>
		int day;

		/// <summary>
		/// The hour
		/// </summary>
		int hour;

		/// <summary>
		/// The minute
		/// </summary>
		int minute;

		/// <summary>
		/// The second
		/// </summary>
		int second;

		/// <summary>
		/// The microsecond
		/// </summary>
		int microsecond;

	};
	std::optional<double> cast_numeric(std::string_view s);
	std::optional<double> cast_percentage(std::string_view s);
	std::optional<int> cast_int(std::string_view s);
	std::optional<time> cast_time(std::string_view s);
}