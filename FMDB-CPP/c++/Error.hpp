//
//  Error.hpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/14.
//  Copyright © 2017年 CHE. All rights reserved.
//

#ifndef Error_hpp
#define Error_hpp

#include "FMDBDefs.h"
#include "Variant.hpp"
#include <unordered_map>
#include <iosfwd>

FMDB_BEGIN

extern const string& LocalizedDescriptionKey;

class Error {
public:
    Error();
    explicit Error(const string &domain, long long code, const Variant &userInfo = Variant::null);

    long long code() const { return _code; }
    const string &domain();

    const Variant& userInfo();
    void setUserInfo(const Variant &userInfo);

    string description() const;
    friend std::ostream& operator<<(std::ostream &os, const Error &error)
    {
        os << error.description();
        return os;
    }

    bool isEmpty() const;
private:
    long long _code;
#if __PL64__
    void *_reserved;
#endif
    std::unique_ptr<string> _domain;
    std::unique_ptr<Variant> _userInfo;
};

FMDB_END

#endif /* Error_hpp */
