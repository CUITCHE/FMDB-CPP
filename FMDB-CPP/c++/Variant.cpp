//
//  Variant.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include "Variant.hpp"
#include <cassert>
#include <sstream>
#include <iomanip>
#include "Date.hpp"

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


Variant::Variant(const VariantData &v)
:_type(Type::DATA)
{
    auto d = new (nothrow) remove_cv<remove_reference<decltype(v)>::type>::type(v);
    _field.object = d;
}

Variant::Variant(VariantData &&v)
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

Variant::Variant(const Variant &other)
{
	*this = other;
}

Variant::Variant(Variant && other)
{
	*this = std::move(other);
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
	switch (_type)
	{
	case FMDB_CPP::Variant::Type::NONE:
		break;
	case FMDB_CPP::Variant::Type::BOOLEAN:return _field.boolVal;
		break;
	case FMDB_CPP::Variant::Type::CHAR: return !!_field.charVal;
		break;
	case FMDB_CPP::Variant::Type::BYTE: return !!_field.byteVal;
		break;
	case FMDB_CPP::Variant::Type::INTEGER: return !!_field.intVal;
		break;
	case FMDB_CPP::Variant::Type::UINTEGER: return !!_field.unsignedVal;
		break;
	case FMDB_CPP::Variant::Type::FLOAT: return !(_field.floatVal == 0);
		break;
	case FMDB_CPP::Variant::Type::DOUBLE: return !(_field.doubleVal == 0);
		break;
	case FMDB_CPP::Variant::Type::LONGLONG: return !(_field.longLongVal == 0);
		break;
	case FMDB_CPP::Variant::Type::ULONGLONG: return !(_field.unsignedLongLongVal == 0);
		break;
	case FMDB_CPP::Variant::Type::STRING: {
		const string &str = *(const string *)_field.object;
		return !(str == "0" || str == "false");
	}
		break;
	case FMDB_CPP::Variant::Type::DATA:
		break;
	case FMDB_CPP::Variant::Type::DATE:
		break;
	case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAP:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
    return false;
}

char Variant::toChar() const
{
    assert(convert(Type::CHAR));
	switch (_type)
	{
	case FMDB_CPP::Variant::Type::NONE:
		break;
	case FMDB_CPP::Variant::Type::BOOLEAN: return static_cast<char>(_field.boolVal);
		break;
	case FMDB_CPP::Variant::Type::CHAR: return _field.charVal;
		break;
	case FMDB_CPP::Variant::Type::BYTE: return _field.byteVal;
		break;
	case FMDB_CPP::Variant::Type::INTEGER: return _field.intVal;
		break;
	case FMDB_CPP::Variant::Type::UINTEGER: return _field.unsignedVal;
		break;
	case FMDB_CPP::Variant::Type::FLOAT: return static_cast<char>(_field.floatVal);
		break;
	case FMDB_CPP::Variant::Type::DOUBLE: return static_cast<char>(_field.doubleVal);
		break;
	case FMDB_CPP::Variant::Type::LONGLONG: return static_cast<char>(_field.longLongVal);
		break;
	case FMDB_CPP::Variant::Type::ULONGLONG: return static_cast<char>(_field.unsignedLongLongVal);
		break;
	case FMDB_CPP::Variant::Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<char>(atoi(str.c_str()));
	}
		break;
	case FMDB_CPP::Variant::Type::DATA:
		break;
	case FMDB_CPP::Variant::Type::DATE:
		break;
	case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAP:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
    return 0;
}

unsigned char Variant::toByte() const
{
	return static_cast<unsigned char>(toChar());
}

int Variant::toInt() const
{
	assert(convert(Type::INTEGER));
	switch (_type)
	{
	case FMDB_CPP::Variant::Type::NONE:
		break;
	case FMDB_CPP::Variant::Type::BOOLEAN: return static_cast<int>(_field.boolVal);
		break;
	case FMDB_CPP::Variant::Type::CHAR: return _field.charVal;
		break;
	case FMDB_CPP::Variant::Type::BYTE: return _field.byteVal;
		break;
	case FMDB_CPP::Variant::Type::INTEGER: return _field.intVal;
		break;
	case FMDB_CPP::Variant::Type::UINTEGER: return _field.unsignedVal;
		break;
	case FMDB_CPP::Variant::Type::FLOAT: return static_cast<int>(_field.floatVal);
		break;
	case FMDB_CPP::Variant::Type::DOUBLE: return static_cast<int>(_field.doubleVal);
		break;
	case FMDB_CPP::Variant::Type::LONGLONG: return static_cast<int>(_field.longLongVal);
		break;
	case FMDB_CPP::Variant::Type::ULONGLONG: return static_cast<int>(_field.unsignedLongLongVal);
		break;
	case FMDB_CPP::Variant::Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<int>(atoi(str.c_str()));
	}
		break;
	case FMDB_CPP::Variant::Type::DATA:
		break;
	case FMDB_CPP::Variant::Type::DATE:
		break;
	case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAP:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return 0;
}

unsigned int Variant::toUInt() const
{
	assert(convert(Type::UINTEGER));
	return static_cast<unsigned int>(toInt());
}

float Variant::toFloat() const
{
	assert(convert(Type::FLOAT));
	return static_cast<float>(toDouble());
}

double Variant::toDouble() const
{
	assert(convert(Type::DOUBLE));
	switch (_type)
	{
	case FMDB_CPP::Variant::Type::NONE:
		break;
	case FMDB_CPP::Variant::Type::BOOLEAN: return static_cast<double>(_field.boolVal);
		break;
	case FMDB_CPP::Variant::Type::CHAR: return _field.charVal;
		break;
	case FMDB_CPP::Variant::Type::BYTE: return _field.byteVal;
		break;
	case FMDB_CPP::Variant::Type::INTEGER: return _field.intVal;
		break;
	case FMDB_CPP::Variant::Type::UINTEGER: return _field.unsignedVal;
		break;
	case FMDB_CPP::Variant::Type::FLOAT: return static_cast<double>(_field.floatVal);
		break;
	case FMDB_CPP::Variant::Type::DOUBLE: return static_cast<double>(_field.doubleVal);
		break;
	case FMDB_CPP::Variant::Type::LONGLONG: return static_cast<double>(_field.longLongVal);
		break;
	case FMDB_CPP::Variant::Type::ULONGLONG: return static_cast<double>(_field.unsignedLongLongVal);
		break;
	case FMDB_CPP::Variant::Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<double>(atof(str.c_str()));
	}
		break;
	case FMDB_CPP::Variant::Type::DATA:
		break;
	case FMDB_CPP::Variant::Type::DATE:
		break;
	case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAP:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return 0.0;
}

long long Variant::toLongLong() const
{
	assert(convert(Type::LONGLONG));
	switch (_type)
	{
	case FMDB_CPP::Variant::Type::NONE:
		break;
	case FMDB_CPP::Variant::Type::BOOLEAN: return static_cast<long long>(_field.boolVal);
		break;
	case FMDB_CPP::Variant::Type::CHAR: return _field.charVal;
		break;
	case FMDB_CPP::Variant::Type::BYTE: return _field.byteVal;
		break;
	case FMDB_CPP::Variant::Type::INTEGER: return _field.intVal;
		break;
	case FMDB_CPP::Variant::Type::UINTEGER: return _field.unsignedVal;
		break;
	case FMDB_CPP::Variant::Type::FLOAT: return static_cast<long long>(_field.floatVal);
		break;
	case FMDB_CPP::Variant::Type::DOUBLE: return static_cast<long long>(_field.doubleVal);
		break;
	case FMDB_CPP::Variant::Type::LONGLONG: return static_cast<long long>(_field.longLongVal);
		break;
	case FMDB_CPP::Variant::Type::ULONGLONG: return static_cast<long long>(_field.unsignedLongLongVal);
		break;
	case FMDB_CPP::Variant::Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<long long>(atoll(str.c_str()));
	}
		break;
	case FMDB_CPP::Variant::Type::DATA:
		break;
	case FMDB_CPP::Variant::Type::DATE:
		break;
	case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAP:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return 0;
}

unsigned long long Variant::toULongLong() const
{
	assert(convert(Type::ULONGLONG));
	return static_cast<unsigned long long>(toLongLong());
}

string Variant::toString() const
{
	assert(convert(Type::STRING));
	if (_type == Type::STRING) {
		return *(string *)_field.object;
	}
	stringstream ss;
	switch (_type)
	{
	case FMDB_CPP::Variant::Type::NONE:
		break;
	case FMDB_CPP::Variant::Type::BOOLEAN: ss << (_field.boolVal ? "true" : "false");
		break;
	case FMDB_CPP::Variant::Type::CHAR: ss << _field.charVal;
		break;
	case FMDB_CPP::Variant::Type::BYTE: ss << _field.byteVal;
		break;
	case FMDB_CPP::Variant::Type::INTEGER: ss << _field.intVal;
		break;
	case FMDB_CPP::Variant::Type::UINTEGER: ss << _field.unsignedVal;
		break;
	case FMDB_CPP::Variant::Type::FLOAT: ss << _field.floatVal;
		break;
	case FMDB_CPP::Variant::Type::DOUBLE: ss << _field.doubleVal;
		break;
	case FMDB_CPP::Variant::Type::LONGLONG: ss << _field.longLongVal;
		break;
	case FMDB_CPP::Variant::Type::ULONGLONG: ss << _field.unsignedLongLongVal;
		break;
	case FMDB_CPP::Variant::Type::STRING:
		break;
	case FMDB_CPP::Variant::Type::DATA:
		break;
	case FMDB_CPP::Variant::Type::DATE:
		break;
	case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAP:
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return ss.str();
}

const Date& Variant::toDate() const
{
	assert(convert(Type::DATE));
	return *static_cast<const Date *>(_field.object);
}

VariantVector & Variant::toVariantVector()
{
	assert(convert(Type::VARIANTVECTOR));
	return *static_cast<VariantVector *>(_field.object);
}

const VariantVector & Variant::toVariantVector() const
{
	assert(convert(Type::VARIANTVECTOR));
	return *static_cast<const VariantVector *>(_field.object);
}

VariantMap & Variant::toVariantMap()
{
	assert(convert(Type::VARIANTMAP));
	return *static_cast<VariantMap *>(_field.object);
}

const VariantMap & Variant::toVariantMap() const
{
	assert(convert(Type::VARIANTMAP));
	return *static_cast<const VariantMap *>(_field.object);
}

VariantMapIntKey & Variant::toVariantMapIntKey()
{
	assert(convert(Type::VARIANTMAPINTKEY));
	return *static_cast<VariantMapIntKey *>(_field.object);
}

const VariantMapIntKey & Variant::toVariantMapIntKey() const
{
	assert(convert(Type::VARIANTMAPINTKEY));
	return *static_cast<const VariantMapIntKey *>(_field.object);
}

VariantData & Variant::toVariantData()
{
	assert(convert(Type::DATA));
	return *static_cast<VariantData *>(_field.object);
}

const VariantData & Variant::toVariantData() const
{
	assert(convert(Type::DATA));
	return *static_cast<const VariantData *>(_field.object);
}

Variant & Variant::operator=(const Variant & other)
{
	if (this != &other) {
		reset(other._type);
		switch (_type) {
		case FMDB_CPP::Variant::Type::NONE:
			break;
		case FMDB_CPP::Variant::Type::BOOLEAN:
		case FMDB_CPP::Variant::Type::CHAR:
		case FMDB_CPP::Variant::Type::BYTE:
		case FMDB_CPP::Variant::Type::INTEGER:
		case FMDB_CPP::Variant::Type::UINTEGER:
		case FMDB_CPP::Variant::Type::FLOAT:
		case FMDB_CPP::Variant::Type::DOUBLE:
		case FMDB_CPP::Variant::Type::LONGLONG:
		case FMDB_CPP::Variant::Type::ULONGLONG:
			_field.unsignedLongLongVal = other._field.unsignedLongLongVal;
			break;
		case FMDB_CPP::Variant::Type::STRING:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) string(*static_cast<string *>(other._field.object));
			} else {
				auto &v = *static_cast<string *>(_field.object);
				v = *static_cast<string *>(other._field.object);
			}
			break;
		case FMDB_CPP::Variant::Type::DATA:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantData (*static_cast<VariantData *>(other._field.object));
			} else {
				auto &v = *static_cast<VariantData *>(_field.object);
				v = *static_cast<VariantData *>(other._field.object);
			}
			break;
		case FMDB_CPP::Variant::Type::DATE:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantData(*static_cast<VariantData *>(other._field.object));
			}
			else {
				auto &v = *static_cast<VariantData *>(_field.object);
				v = *static_cast<VariantData *>(other._field.object);
			}
			break;
		case FMDB_CPP::Variant::Type::VARIANTVECTOR:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantVector(*static_cast<VariantVector *>(other._field.object));
			}
			else {
				auto &v = *static_cast<VariantVector *>(_field.object);
				v = *static_cast<VariantVector *>(other._field.object);
			}
			break;
		case FMDB_CPP::Variant::Type::VARIANTMAP:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantMap(*static_cast<VariantMap *>(other._field.object));
			}
			else {
				auto &v = *static_cast<VariantMap *>(_field.object);
				v = *static_cast<VariantMap *>(other._field.object);
			}
			break;
		case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantMapIntKey(*static_cast<VariantMapIntKey *>(other._field.object));
			}
			else {
				auto &v = *static_cast<VariantMapIntKey *>(_field.object);
				v = *static_cast<VariantMapIntKey *>(other._field.object);
			}
			break;
		default:
			break;
		}
	}
	return *this;
}

Variant & Variant::operator=(Variant && other)
{
	if (this != &other) {
		clear();
		_type = other._type;
		_field = other._field;
		memset(&other._field, 0, sizeof(other._field));
		other._type = Type::NONE;
	}
	return *this;
}

Variant & Variant::operator=(unsigned char v)
{
	reset(Type::BYTE);
	_field.byteVal = v;
	return *this;
}

Variant & Variant::operator=(int v)
{
	reset(Type::INTEGER);
	_field.intVal = v;
	return *this;
}

Variant & Variant::operator=(unsigned int v)
{
	reset(Type::UINTEGER);
	_field.unsignedVal = v;
	return *this;
}

Variant & Variant::operator=(float v)
{
	reset(Type::FLOAT);
	_field.floatVal = v;
	return *this;
}

Variant & Variant::operator=(double v)
{
	reset(Type::DOUBLE);
	_field.doubleVal = v;
	return *this;
}

Variant & Variant::operator=(bool v)
{
	reset(Type::BOOLEAN);
	_field.boolVal = v;
	return *this;
}

Variant & Variant::operator=(const char * v)
{
	reset(Type::STRING);
	*static_cast<string *>(_field.object) = v ? v : "";
	return *this;
}

Variant & Variant::operator=(const std::string & v)
{
	reset(Type::STRING);
	*static_cast<string *>(_field.object) = v;
	return *this;
}

Variant & Variant::operator=(const Date & v)
{
	reset(Type::DATE);
	*static_cast<Date *>(_field.object) = v;
	return *this;
}

Variant & Variant::operator=(const VariantVector & v)
{
	reset(Type::VARIANTVECTOR);
	*static_cast<VariantVector *>(_field.object) = v;
	return *this;
}

Variant & Variant::operator=(VariantVector && v)
{
	reset(Type::VARIANTVECTOR);
	*static_cast<VariantVector *>(_field.object) = std::move(v);
	return *this;
}

Variant & Variant::operator=(const VariantMap & v)
{
	reset(Type::VARIANTMAP);
	*static_cast<VariantMap *>(_field.object) = v;
	return *this;
}

Variant & Variant::operator=(VariantMap && v)
{
	reset(Type::VARIANTMAP);
	*static_cast<VariantMap *>(_field.object) = std::move(v);
	return *this;
}

Variant & Variant::operator=(const VariantMapIntKey & v)
{
	reset(Type::VARIANTMAPINTKEY);
	*static_cast<VariantMapIntKey *>(_field.object) = v;
	return *this;
}

Variant & Variant::operator=(VariantMapIntKey && v)
{
	reset(Type::VARIANTMAPINTKEY);
	*static_cast<VariantMapIntKey *>(_field.object) = std::move(v);
	return *this;
}

Variant & Variant::operator=(const VariantData & v)
{
	reset(Type::DATA);
	*static_cast<VariantData *>(_field.object) = v;
	return *this;
}

Variant & Variant::operator=(VariantData && v)
{
	reset(Type::DATA);
	*static_cast<VariantData *>(_field.object) = std::move(v);
	return *this;
}

bool Variant::operator!=(const Variant & v)
{
	return !(*this == v);
}

bool Variant::operator!=(const Variant & v) const
{
	return !(*this == v);
}

bool Variant::operator==(const Variant & v)
{
	const auto &t = *this;
	return t == v;
}

bool Variant::operator==(const Variant & v) const
{
	if (this == &v) {
		return true;
	}
	if (_type != v._type) {
		return false;
	}
	if (isNull()) {
		return true;
	}
	switch (_type)
	{
	case FMDB_CPP::Variant::Type::NONE:
		break;
	case FMDB_CPP::Variant::Type::BOOLEAN:
	case FMDB_CPP::Variant::Type::CHAR:
	case FMDB_CPP::Variant::Type::BYTE:
	case FMDB_CPP::Variant::Type::INTEGER:
	case FMDB_CPP::Variant::Type::UINTEGER:
		return _field.unsignedVal == v._field.unsignedVal;
	case FMDB_CPP::Variant::Type::FLOAT:
		return std::abs(v._field.floatVal - _field.floatVal) <= FLT_EPSILON;
	case FMDB_CPP::Variant::Type::DOUBLE:
		return std::abs(v._field.doubleVal - _field.doubleVal) <= FLT_EPSILON;
	case FMDB_CPP::Variant::Type::LONGLONG:
	case FMDB_CPP::Variant::Type::ULONGLONG:
		return _field.unsignedLongLongVal == v._field.unsignedLongLongVal;
		break;
	case FMDB_CPP::Variant::Type::STRING:
		return *static_cast<string *>(_field.object) == *static_cast<string *>(v._field.object);
		break;
	case FMDB_CPP::Variant::Type::DATA:
		return *static_cast<VariantData *>(_field.object) == *static_cast<VariantData *>(v._field.object);
		break;
	case FMDB_CPP::Variant::Type::DATE:
		return *static_cast<Date *>(_field.object) == *static_cast<Date *>(v._field.object);
		break;
	case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		return *static_cast<VariantVector *>(_field.object) == *static_cast<VariantVector *>(v._field.object);
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAP: {
		const auto &m1 = *static_cast<VariantMap *>(_field.object);
		const auto &m2 = *static_cast<VariantMap *>(v._field.object);
		if (m1.size() != m2.size()) {
			return false;
		}
		for (auto &iter : m1) {
			auto it = m2.find(iter.first);
			if (it == m2.end() || it->second != iter.second) {
				return false;;
			}
		}
		return true;
	}
		break;
	case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY: {
		const auto &m1 = *static_cast<VariantMapIntKey *>(_field.object);
		const auto &m2 = *static_cast<VariantMapIntKey *>(v._field.object);
		if (m1.size() != m2.size()) {
			return false;
		}
		for (auto &iter : m1) {
			auto it = m2.find(iter.first);
			if (it == m2.end() || it->second != iter.second) {
				return false;;
			}
		}
		return true;
	}
		break;
	default:
		break;
	}
	return false;
}


#define _SAFE_DELETE(p)   do { delete (p); (p) = nullptr; } while(0)
#define SAFE_DELETE(type) do { auto p = (type)_field.object; _SAFE_DELETE(p); } while(0)

void Variant::clear()
{
    if (_type < Type::STRING) {
        _field.unsignedLongLongVal = 0;
    } else if (_type == Type::STRING) {
        SAFE_DELETE(string *);
    } else if (_type == Type::DATA) {
        SAFE_DELETE(VariantData *);
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
        _field.object = new (nothrow) VariantData;
    } else if (_type == Type::DATE) {
        _field.object = new (nothrow) Date(seconds(0));
    } else if (_type == Type::VARIANTVECTOR) {
        _field.object = new (nothrow) VariantVector;
    } else if (_type == Type::VARIANTMAP) {
        _field.object = new (nothrow) VariantMap;
    } else if (_type == Type::VARIANTMAPINTKEY) {
        _field.object = new (nothrow) VariantMapIntKey;
    }
	_type = type;
}

FMDB_END
