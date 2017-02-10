//
//  Variant.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include "Variant.hpp"
#include "Date.hpp"
#include <cassert>

using namespace std;

FMDB_BEGIN

Variant::Variant()
:_type(Type::NONE)
{
}

Variant::Variant(bool v)
:_type(Type::BOOLEAN)
{
    _field.boolVal = v;
}

Variant::Variant(char v)
:_type(Type::CHAR)
{
    _field.charVal = v;
}

Variant::Variant(unsigned char v)
:_type(Type::BYTE)
{
    _field.byteVal = v;
}

Variant::Variant(int v)
:_type(Type::INTEGER)
{
    _field.intVal = v;
}

Variant::Variant(unsigned int v)
:_type(Type::UINTEGER)
{
    _field.unsignedVal = v;
}

Variant::Variant(float v)
:_type(Type::FLOAT)
{
    _field.floatVal = v;
}

Variant::Variant(double v)
:_type(Type::DOUBLE)
{
    _field.doubleVal = v;
}

Variant::Variant(long long v)
:_type(Type::LONGLONG)
{
    _field.longLongVal = v;
}

Variant::Variant(unsigned long long v)
:_type(Type::ULONGLONG)
{
    _field.unsignedLongLongVal = v;
}


Variant::Variant(const vector<unsigned char> &v)
:_type(Type::DATA)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}

Variant::Variant(const vector<unsigned char> &&v)
:_type(Type::DATA)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(move(v));
    _field.object = d;
}


Variant::Variant(const Date &v)
:_type(Type::DATE)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}


Variant::Variant(const char *v)
:_type(Type::STRING)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}

Variant::Variant(const string &v)
:_type(Type::STRING)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}

Variant::Variant(string &&v)
:_type(Type::STRING)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(move(v));
    _field.object = d;
}


Variant::Variant(const VariantVector &v)
:_type(Type::VARIANTVECTOR)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}

Variant::Variant(VariantVector &&v)
:_type(Type::VARIANTVECTOR)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(move(v));
    _field.object = d;
}


Variant::Variant(const VariantMap &v)
:_type(Type::VARIANTMAP)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}

Variant::Variant(VariantMap &&v)
:_type(Type::VARIANTMAP)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(move(v));
    _field.object = d;
}


Variant::Variant(const VariantMapIntKey &v)
:_type(Type::VARIANTMAPINTKEY)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}

Variant::Variant(VariantMapIntKey &&v)
:_type(Type::VARIANTMAPINTKEY)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(move(v));
    _field.object = d;
}

Variant::~Variant()
{
    clear();
}

bool Variant::convert(Type toType) const
{
    if (_type == Type::NONE) {
        return false;
    }
    if (_type == toType) {
        return true;
    }
    if (toType >= Type::DATA || _type >= Type::DATA) { // Data以上的数据结构不能够被转换
        return false;
    }
    return true;
}

bool Variant::toBool() const
{
    assert(convert(Type::BOOLEAN));
    switch (_type) {
        case Type::BOOLEAN:
            return _field.boolVal;
            break;
        case Type::CHAR:
            return !!_field.charVal;
        case Type::BYTE:
            return !!_field.byteVal;
        default:
            break;
    }
    if (_type < Type::STRING) {
        return _field.boolVal;
    } else if (_type == Type::STRING) {
        const string &str = *(const string *)_field.object;
        return !(str == "0" || str == "false");
    }
    return false;
}

char Variant::toChar() const
{
    assert(convert(Type::CHAR));
    if (_type < Type::STRING) {
        return _field.charVal;
    } else if (_type == Type::STRING) {
        const string &str = *(const string *)_field.object;
        return static_cast<char>(atoi(str.c_str()));
    }
    return 0;
}

unsigned char Variant::toByte() const
{
    assert(convert(Type::BYTE));
    if (_type < Type::STRING) {
        return _field.byteVal;
    } else if (_type == Type::STRING) {
        const string &str = *(const string *)_field.object;
        return static_cast<unsigned char>(atoi(str.c_str()));
    }
    return 0;
}

//int Variant::toInt() const;
//unsigned int Variant::toUInt() const;
//float Variant::toFloat() const;
//double Variant::toDouble() const;
//long long Variant::toLongLong() const;
//unsigned long long Variant::toULongLong() const;
//string Variant::toString() const;
//string Variant::description() const
//{
//    ;
//}

#define _SAFE_DELETE(p)           do { delete (p); (p) = nullptr; } while(0)
#define SAFE_DELETE(type) do { auto p = (type)_field.object; _SAFE_DELETE(p); } while(0)

void Variant::clear()
{
    if (_type < Type::STRING) {
        _field.unsignedLongLongVal = 0;
    } else if (_type == Type::STRING) {
        SAFE_DELETE(string *);
    } else if (_type == Type::DATA) {
        SAFE_DELETE(vector<unsigned char> *);
    } else if (_type == Type::DATE) {
        SAFE_DELETE(Date *);
    } else if (_type == Type::VARIANTVECTOR) {
        SAFE_DELETE(VariantVector *);
    } else if (_type == Type::VARIANTMAP) {
        SAFE_DELETE(VariantMap *);
    } else if (_type == Type::VARIANTMAPINTKEY) {
        SAFE_DELETE(VariantMapIntKey *);
    }

    _type = Type::NONE;
}

void Variant::reset(Type type)
{
    if (_type == type) {
        return;
    }
    clear();
    if (_type == Type::STRING) {
        _field.object = new (nothrow) string;
    } else if (_type == Type::DATA) {
        _field.object = new (nothrow) vector<unsigned char>;
    } else if (_type == Type::DATE) {
        _field.object = new (nothrow) Date(seconds(0));
    } else if (_type == Type::VARIANTVECTOR) {
        _field.object = new (nothrow) VariantVector;
    } else if (_type == Type::VARIANTMAP) {
        _field.object = new (nothrow) VariantMap;
    } else if (_type == Type::VARIANTMAPINTKEY) {
        _field.object = new (nothrow) VariantMapIntKey;
    }
}



FMDB_END
