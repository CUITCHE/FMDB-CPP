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

using TimeInterval = std::chrono::duration<double>;

extern const TimeInterval TimeIntervalSince1970;

class Date {
    std::chrono::system_clock::time_point d;
    explicit Date(int);
public:
    explicit Date();
    explicit Date(TimeInterval SinceReferenceDate);
    ~Date();
    Date clone();

    TimeInterval timeIntervalSinceDate(Date anotherData) const;

    TimeInterval timeIntervalSinceNow() const;
    TimeInterval timeIntervalSince1970() const;
    TimeInterval timeIntervalSinceReferenceDate() const;

    Date dateByAddingTimeInterval(TimeInterval seconds) const;

    static Date dateWithTimeIntervalSinceNow(TimeInterval secs);
    static Date dateWithTimeIntervalSince1970(TimeInterval secs);
    static Date dateWithTimeIntervalSinceReferenceDate(TimeInterval ti);
    static Date dateWithTimeInterval(TimeInterval secsToAdded, Date sinceDate);

};

#endif /* Date_hpp */
