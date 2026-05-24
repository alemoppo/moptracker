#include "utils.h"

int64_t unix_now() {
    return static_cast<int64_t>(std::time(nullptr));
}

struct tm local_time(int64_t ts) {
    time_t t = static_cast<time_t>(ts);
    struct tm result;
#ifdef _WIN32
    localtime_s(&result, &t);
#else
    localtime_r(&t, &result);
#endif
    return result;
}

int64_t make_timestamp(int year, int mon, int day, int hour, int min, int sec) {
    struct tm tm = {};
    tm.tm_year = year - EPOCH_YEAR_OFFSET;
    tm.tm_mon  = mon - TM_MON_ADJUST;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min  = min;
    tm.tm_sec  = sec;
    tm.tm_isdst = -1;
    return static_cast<int64_t>(std::mktime(&tm));
}

int64_t day_start_ts(int64_t ts) {
    struct tm lt = local_time(ts);
    return make_timestamp(lt.tm_year + EPOCH_YEAR_OFFSET, lt.tm_mon + TM_MON_ADJUST, lt.tm_mday, 0, 0, 0);
}

int64_t day_end_ts(int64_t ts) {
    struct tm lt = local_time(ts);
    return make_timestamp(lt.tm_year + EPOCH_YEAR_OFFSET, lt.tm_mon + TM_MON_ADJUST, lt.tm_mday, END_OF_DAY_HOUR, END_OF_DAY_MINUTE, END_OF_DAY_SECOND);
}

int64_t month_start_ts(int year, int mon) {
    return make_timestamp(year, mon, 1, 0, 0, 0);
}

int64_t month_end_ts(int year, int mon) {
    return make_timestamp(year, mon, days_in_month(year, mon), END_OF_DAY_HOUR, END_OF_DAY_MINUTE, END_OF_DAY_SECOND);
}

int days_in_month(int year, int mon) {
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (mon == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
        return 29;
    return days[mon - 1];
}

std::string format_timestamp(int64_t ts) {
    struct tm lt = local_time(ts);
    char buf[FMT_TS_BUF];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        lt.tm_year + EPOCH_YEAR_OFFSET, lt.tm_mon + TM_MON_ADJUST, lt.tm_mday,
        lt.tm_hour, lt.tm_min, lt.tm_sec);
    return buf;
}

std::string format_duration(int64_t seconds) {
    if (seconds < 0) seconds = 0;
    int h = static_cast<int>(seconds / SECONDS_PER_HOUR);
    int m = static_cast<int>((seconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
    int s = static_cast<int>(seconds % SECONDS_PER_MINUTE);
    char buf[FMT_DUR_BUF];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    return buf;
}

std::string format_date(int64_t ts) {
    struct tm lt = local_time(ts);
    char buf[FMT_DATE_BUF];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
        lt.tm_year + EPOCH_YEAR_OFFSET, lt.tm_mon + TM_MON_ADJUST, lt.tm_mday);
    return buf;
}

std::string format_month(int year, int mon, int lang) {
    static const char* months_en[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    static const char* months_it[] = {
        "Gennaio", "Febbraio", "Marzo", "Aprile", "Maggio", "Giugno",
        "Luglio", "Agosto", "Settembre", "Ottobre", "Novembre", "Dicembre"
    };
    const char** months = (lang == 1) ? months_it : months_en;
    char buf[FMT_MONTH_BUF];
    snprintf(buf, sizeof(buf), "%s %04d", months[mon - 1], year);
    return buf;
}
