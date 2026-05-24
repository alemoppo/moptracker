#pragma once
#include <cstdint>
#include <string>
#include <ctime>

// Time constants
inline constexpr int    EPOCH_YEAR_OFFSET  = 1900;
inline constexpr int    TM_MON_ADJUST      = 1;
inline constexpr int    END_OF_DAY_HOUR    = 23;
inline constexpr int    END_OF_DAY_MINUTE  = 59;
inline constexpr int    END_OF_DAY_SECOND  = 59;
inline constexpr int    SECONDS_PER_HOUR   = 3600;
inline constexpr int    SECONDS_PER_MINUTE = 60;
inline constexpr int    SECONDS_PER_DAY    = 86400;

// Format buffer sizes
inline constexpr int    FMT_TS_BUF = 64;
inline constexpr int    FMT_DUR_BUF = 32;
inline constexpr int    FMT_DATE_BUF = 16;
inline constexpr int    FMT_MONTH_BUF = 32;

int64_t  unix_now();
struct tm local_time(int64_t ts);
int64_t  make_timestamp(int year, int mon, int day, int hour, int min, int sec);
int64_t  day_start_ts(int64_t ts);
int64_t  day_end_ts(int64_t ts);
int64_t  month_start_ts(int year, int mon);
int64_t  month_end_ts(int year, int mon);
int      days_in_month(int year, int mon);
std::string format_timestamp(int64_t ts);
std::string format_duration(int64_t seconds);
std::string format_date(int64_t ts);
std::string format_month(int year, int mon, int lang = 0);
