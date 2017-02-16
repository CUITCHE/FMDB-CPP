//
//  Date.hpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#ifndef Date_hpp
#define Date_hpp

#include "FMDBDefs.h"
#include <iosfwd>

FMDB_BEGIN

extern const TimeInterval TimeIntervalSince1970;

class Date {
    system_clock::time_point d;
public:

    explicit Date(system_clock::time_point);

    /**
     Creates and returns a new date set to the current date and time.
     */
    Date();

    /**
     Creates and returns an NSDate object set to a given number of seconds from 00:00:00 UTC on 1 January 2001.

     @param SinceReferenceDate The number of seconds from the absolute reference date (00:00:00 UTC on 1 January 2001) for the new date. Use a negative argument to specify a date and time before the reference date.
     */
    explicit Date(TimeInterval SinceReferenceDate);
    ~Date();


    /**
     Returns a copy date that is same to this date.

     @return A copy date.
     */
    Date clone();


    /**
     Returns the interval between this and another given date.

     @param anotherDate The date with which to compare this.
     @return The interval between this and the anotherDate parameter. If the receiver is earlier than anotherDate, the return value is negative.
     */
    TimeInterval timeIntervalSinceDate(Date anotherDate) const;


    /**
     The time interval between the date object and the current date and time.
     If the date object is earlier than the current date and time, this value is negative.

     @return The interval between this and the current date.
     */
    TimeInterval timeIntervalSinceNow() const;

    /**
     The interval between the date object and 00:00:00 UTC on 1 January 1970.
     This value is negative if the date object is earlier than 00:00:00 UTC on 1 January 1970.

     @return The interval between this and the date(00:00:00 UTC on 1 January 1970).
     */
    TimeInterval timeIntervalSince1970() const;

    /**
     Returns the interval between the date object and 00:00:00 UTC on 1 January 2001.
     This value is negative if the date object is earlier than the system’s absolute reference date (00:00:00 UTC on 1 January 2001).

     @return The interval between this and the date(00:00:00 UTC on 1 January 2001).
     */
    TimeInterval timeIntervalSinceReferenceDate() const;


    /**
     Returns a new Date object that is set to a given number of seconds relative to the receiver.

     @param seconds The number of seconds to add to this. Use a negative value for seconds to have the returned object specify a date before this date.
     @return A new Date object that is set to seconds seconds relative to this. The date returned might have a representation different from this.
     */
    Date dateByAddingTimeInterval(TimeInterval seconds) const;

    static Date dateWithTimeIntervalSinceNow(TimeInterval secs);
    static Date dateWithTimeIntervalSince1970(TimeInterval secs);
    static Date dateWithTimeIntervalSinceReferenceDate(TimeInterval ti);
    static Date dateWithTimeInterval(TimeInterval secsToAdded, Date sinceDate);
    static Date dateFromString(const std::string &dateString);

    static string stringFromDate(const Date &date);

    string description() const;

    /* operators */

    /**
     Add clocks to date.
     @param clocks clock type value. nanoseconds, microseconds, milliseconds, seconds, minutes, hours or any duration else.
     @return return self
     */
    template<typename clock_type>
    Date& operator += (const clock_type &clocks)
    {
        d += clocks;
        return *this;
    }

    /**
     Minus clocks to date.
     @param clocks clock type value. nanoseconds, microseconds, milliseconds, seconds, minutes, hours or any duration else.
     @return return self
     */
    template<typename clock_type>
    Date& operator -= (const clock_type &clocks)
    {
        d -= clocks;
        return *this;
    }

    /**
     Create a new date which has been minused clocks from this date.
     @param clocks clock type value. nanoseconds, microseconds, milliseconds, seconds, minutes, hours or any duration else.
     @return return self
     */
    template<typename clock_type>
    Date operator - (const clock_type &clocks)
    {
        auto copy = clone();
        copy.d -= clocks;
        return copy;
    }

    /**
     Create a new date which has been added clocks from this date.
     @param clocks clock type value. nanoseconds, microseconds, milliseconds, seconds, minutes, hours or any duration else.
     @return return self
     */
    template<typename clock_type>
    Date operator + (const clock_type &clocks)
    {
        auto copy = clone();
        copy.d += clocks;
        return copy;
    }

	bool operator==(const Date &rhs) const { return this->d == rhs.d; }
	bool operator!=(const Date &rhs) const { return !(*this == rhs); }

	friend std::ostream& operator<<(std::ostream &os, const Date &date)
    {
        os << date.description();
        return os;
    }
};

FMDB_END

#endif /* Date_hpp */
