//
//  Date.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include "Date.hpp"

using namespace std::chrono;

const TimeInterval TimeIntervalSince1970 = TimeInterval(978307200.0);

Date::Date()
:d(system_clock::now())
{
}

Date::Date(int)
:d(duration_cast<system_clock::duration>(TimeInterval(0)))
{
}

Date::Date(TimeInterval SinceReferenceDate)
:d(duration_cast<system_clock::duration>(TimeIntervalSince1970 + SinceReferenceDate))
{
}

Date Date::clone()
{
    Date date(1);
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
    Date date(1);
    date.d += duration_cast<system_clock::duration>(secs);
    return date;
}

Date Date::dateWithTimeIntervalSinceReferenceDate(TimeInterval ti)
{
    Date date(0);
    date.d += duration_cast<system_clock::duration>(TimeIntervalSince1970 + ti);
    return date;
}

Date Date::dateWithTimeInterval(TimeInterval secsToAdded, Date sinceDate)
{
    sinceDate.d += duration_cast<system_clock::duration>(secsToAdded);
    return sinceDate;
}
