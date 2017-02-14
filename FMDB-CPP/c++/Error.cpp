//
//  Error.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/14.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include "Error.hpp"


using namespace std;

FMDB_BEGIN

const string& LocalizedDescriptionKey = "LocalizedDescription";

Error::Error()
:_code(0)
,_domain(nullptr)
,_userInfo(nullptr)
{
}

Error::Error(const string &domain, long long code, const Variant &userInfo)
:_code(code)
,_domain(new string(domain))
,_userInfo(new Variant(userInfo))
{
}

const string& Error::domain()
{
    if (!_domain) {
        _domain.reset(new string);
    }

    return *_domain;
}

const Variant& Error::userInfo()
{
    if (!_userInfo) {
        _userInfo.reset(new Variant);
    }
    return *_userInfo;
}

void Error::setUserInfo(const Variant &userInfo)
{
    if (_userInfo) {
        *_userInfo = userInfo;
    } else {
        _userInfo.reset(new Variant(userInfo));
    }
}

string Error::description() const
{
    string desc("Error Domain=");
    desc.append(*_domain).append(" Code=").append(Variant(_code).toString());
    if (_userInfo) {
        if (_userInfo->getType() == Variant::Type::VARIANTMAP) {
            desc.append(" ");
            auto &map = _userInfo->toVariantMap();
            auto iter = map.find(LocalizedDescriptionKey);
            if (iter != map.end()) {
                desc.append(iter->second.description());
            } else {
                desc.append("(null)");
            }
            desc.append(" ");
        }
    }
    desc.append("UserInfo=");
    if (_userInfo) {
        desc.append(_userInfo->description());
    } else {
        desc.append("(null)");
    }
    return desc;
}

bool Error::isEmpty() const
{
    return !(_domain && _userInfo);
}

FMDB_END
