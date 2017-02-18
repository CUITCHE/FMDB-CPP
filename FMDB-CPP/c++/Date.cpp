//
//  Date.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include "Date.hpp"

FMDB_BEGIN

const TimeInterval TimeIntervalSince1970 = TimeInterval(978307200.0);
const char *const FMDB_CPP_DATE_FORMAT = "%a %b %d %Y %H:%M:%S %z";

Date::Date()
:d(system_clock::now())
{
}

Date::Date(time_point tp)
:d(tp)
{
}

Date::Date(TimeInterval SinceReferenceDate)
:d(duration_cast<system_clock::duration>(TimeIntervalSince1970 + SinceReferenceDate))
{
}

Date Date::clone()
{
    time_point t;
    Date date(t);
    date.d = d;
    return date;
}

Date::~Date() { }

TimeInterval Date::timeIntervalSinceDate(Date anotherData) const
{
    return d.time_since_epoch() - anotherData.d.time_since_epoch();
}

TimeInterval Date::timeIntervalSinceNow() const
{
    return d - system_clock::now();
}

TimeInterval Date::timeIntervalSince1970() const
{
    return d.time_since_epoch();
}

TimeInterval Date::timeIntervalSinceReferenceDate() const
{
    return d.time_since_epoch() - TimeIntervalSince1970;
}

Date Date::dateByAddingTimeInterval(TimeInterval seconds) const
{
    Date date;
    date.d += duration_cast<system_clock::duration>(seconds);
    return date;
}

Date Date::dateWithTimeIntervalSinceNow(TimeInterval secs)
{
    Date date;
    date.d += duration_cast<system_clock::duration>(secs);
    return date;
}

Date Date::dateWithTimeIntervalSince1970(TimeInterval secs)
{
    time_point t;
    Date date(t);
    date.d += duration_cast<system_clock::duration>(secs);
    return date;
}

Date Date::dateWithTimeIntervalSinceReferenceDate(TimeInterval ti)
{
    time_point t;
    Date date(t);
    date.d += duration_cast<system_clock::duration>(TimeIntervalSince1970 + ti);
    return date;
}

Date Date::dateWithTimeInterval(TimeInterval secsToAdded, Date sinceDate)
{
    sinceDate.d += duration_cast<system_clock::duration>(secsToAdded);
    return sinceDate;
}

Date Date::dateEpoch()
{
    return dateWithTimeIntervalSince1970(TimeInterval(0));
}

#ifdef WIN32

void strptime(const char *buf, const char *format, struct tm *tm)
{
	_assert(format == FMDB_CPP_DATE_FORMAT, "TODO:");
	// TODO:
}

#endif

Date Date::dateFromString(const string &dateString)
{
    struct tm tm = {0};
    strptime(dateString.c_str(), FMDB_CPP_DATE_FORMAT, &tm);

    struct tm validate_tm{0};
    if (memcmp(&tm, &validate_tm, sizeof(struct tm)) == 0) {
        double count = atof(dateString.c_str());
        Date date = dateEpoch();
        date += TimeInterval(count);
        return date;
    }

    time_point t;
    Date date(t);
    date.d = system_clock::from_time_t(mktime(&tm));
    return date;
}

string Date::stringFromDate(const Date &date)
{
    time_t time = system_clock::to_time_t(date.d);
    char str[64];
    struct tm tm;
#ifdef WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    size_t length = strftime(str, sizeof(str), FMDB_CPP_DATE_FORMAT, &tm);
    return string(str, str + length);
}

string Date::description() const
{
    return Date::stringFromDate(*this);
}

FMDB_END
