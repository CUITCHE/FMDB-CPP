//
//  Variant.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include "Variant.hpp"
#include <sstream>
#include <iomanip>
#include <cfloat>
#include "Date.hpp"

using namespace std;

FMDB_BEGIN

const Variant Variant::null;

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
    auto d = new (nothrow) VariantData(v);
    _field.object = d;
}

Variant::Variant(VariantData &&v)
:_type(Type::DATA)
{
    auto d = new (nothrow) VariantData(move(v));
    _field.object = d;
}


Variant::Variant(const Date &v)
:_type(Type::DATE)
{
    auto d = new (nothrow) Date(v);
    _field.object = d;
}


Variant::Variant(const char *v)
:_type(Type::STRING)
{
    if (v) {
        auto d = new (nothrow) string(v);
        _field.object = d;
    } else {
        _type = Type::NONE;
    }
}

Variant::Variant(const string &v)
:_type(Type::STRING)
{
    auto d = new (nothrow) string(v);
    _field.object = d;
}

Variant::Variant(string &&v)
:_type(Type::STRING)
{
    auto d = new (nothrow) string(move(v));
    _field.object = d;
}


Variant::Variant(const VariantVector &v)
:_type(Type::VARIANTVECTOR)
{
    auto d = new (nothrow) VariantVector(v);
    _field.object = d;
}

Variant::Variant(VariantVector &&v)
:_type(Type::VARIANTVECTOR)
{
    auto d = new (nothrow) VariantVector(move(v));
    _field.object = d;
}


Variant::Variant(const VariantMap &v)
:_type(Type::VARIANTMAP)
{
    auto d = new (nothrow) VariantMap(v);
    _field.object = d;
}

Variant::Variant(VariantMap &&v)
:_type(Type::VARIANTMAP)
{
    auto d = new (nothrow) VariantMap(move(v));
    _field.object = d;
}


Variant::Variant(const VariantMapIntKey &v)
:_type(Type::VARIANTMAPINTKEY)
{
    auto d = new (nothrow) VariantMapIntKey(v);
    _field.object = d;
}

Variant::Variant(VariantMapIntKey &&v)
:_type(Type::VARIANTMAPINTKEY)
{
    auto d = new (nothrow) VariantMapIntKey(move(v));
    _field.object = d;
}

Variant::Variant(const Variant &other)
:_type(Type::NONE)
{
	*this = other;
}

Variant::Variant(Variant && other)
:_type(Type::NONE)
{
	*this = std::move(other);
}

Variant::Variant(std::nullptr_t)
:_type(Type::NONE)
{
}

Variant::~Variant()
{
    fprintf(stderr, "Destructor：%p(%s)\n", this, description().c_str());
    fflush(stderr);
    clear();
}

static std::string getTabs(int depth)
{
    std::string tabWidth;

    for (int i = 0; i < depth; ++i)
    {
        tabWidth += "\t";
    }

    return tabWidth;
}

static string visit(const Variant &v, int depth);

static string visitVector(const VariantVector &v, int depth)
{
    stringstream ss;
    if (depth > 0) {
        ss << "\n";
    }

    ss << getTabs(depth) << "[";

    int i = 0;
    for (const auto &child : v) {
        ss << getTabs(depth + 1) << visit(child, depth + 1);
        if (++i < v.size()) {
            ss << ",\n";
        }
    }
    ss << getTabs(depth) << "]\n";
    return ss.str();
}

template <typename T>
static string visitMap(const T &v, int depth)
{
    stringstream ss;
    if (depth > 0) {
        ss << "\n";
    }

    ss << getTabs(depth) << "{";
    if (std::is_base_of<string, typename T::key_type>::value) {
        for (auto &iter : v) {
            ss << getTabs(depth + 1) << "\"" << iter.first << "\" = ";
            ss << visit(iter.second, depth + 1);
        }
    } else {
        for (auto &iter : v) {
            ss << getTabs(depth + 1) << iter.first << " = ";
            ss << visit(iter.second, depth + 1);
        }
    }

    ss << getTabs(depth + 1) << "}\n";

    return ss.str();
}

static string visit(const Variant &v, int depth)
{
    stringstream ss;
    switch (v.getType()) {
        case Variant::Type::NONE:
        case Variant::Type::BOOLEAN:
        case Variant::Type::CHAR:
        case Variant::Type::BYTE:
        case Variant::Type::INTEGER:
        case Variant::Type::UINTEGER:
        case Variant::Type::FLOAT:
        case Variant::Type::DOUBLE:
        case Variant::Type::LONGLONG:
        case Variant::Type::ULONGLONG:
            ss << const_cast<Variant &>(v).toString() << "\n";
            break;
        case Variant::Type::STRING:
            ss << "\"" <<const_cast<Variant &>(v).toString() << "\"" << "\n";
            break;
        case Variant::Type::DATA: {
            auto &data = v.toVariantData();
            const unsigned char *d = data.data();
            size_t length = data.size();
            const long long *pll = (const long long *)d;
            ss.setf(ios::showbase);
            ss.setf(ios_base::hex, ios_base::basefield);
            while (length >= 8) {
                ss << "0x" << *pll++ << " ";
                length -= 8;
            }
            if (length > 0) {
                long long rest = 0;
                memcpy(&rest, pll, length);
                ss << "0x" << rest;

            } else if (length == 0){
                string str = ss.str();
                str.erase(str.length() - 1, 1);
                return str;
            } else {
                _assert(false, "length(%zu) must be >= 0.", length);
            }
        }
            break;
        case Variant::Type::DATE:
            ss << v.toDate();
            break;
        case Variant::Type::VARIANTVECTOR:
            ss << visitVector(v.toVariantVector(), depth);
            break;
        case Variant::Type::VARIANTMAP:
            ss << visitMap(v.toVariantMap(), depth);
            break;
        case Variant::Type::VARIANTMAPINTKEY:
            ss << visitMap(v.toVariantMapIntKey(), depth);
            break;
//        default:
//            break;
    }
    return ss.str();
}

string Variant::description() const
{
    string str = visit(*this, 0);
    return str;
}

bool Variant::convert(Type toType) const
{
    if (_type == Type::NONE) {
        if (toType == Type::STRING) {
            return true;
        }
        return false;
    }
    if (_type == toType) {
        return true;
    }
    if (_type == Type::DATE && toType == Type::STRING) {
        return true;
    }
    if (toType >= Type::DATA || _type >= Type::DATA) { // Data以上的数据结构不能够被转换
        return false;
    }
    return true;
}

bool Variant::toBool() const
{
    _assert(convert(Type::BOOLEAN), "Can't convert to boolea");
	switch (_type)
	{
	case Type::NONE:
		break;
	case Type::BOOLEAN:return _field.boolVal;
		break;
	case Type::CHAR: return !!_field.charVal;
		break;
	case Type::BYTE: return !!_field.byteVal;
		break;
	case Type::INTEGER: return !!_field.intVal;
		break;
	case Type::UINTEGER: return !!_field.unsignedVal;
		break;
	case Type::FLOAT: return !(_field.floatVal == 0);
		break;
	case Type::DOUBLE: return !(_field.doubleVal == 0);
		break;
	case Type::LONGLONG: return !(_field.longLongVal == 0);
		break;
	case Type::ULONGLONG: return !(_field.unsignedLongLongVal == 0);
		break;
	case Type::STRING: {
		const string &str = *(const string *)_field.object;
		return !(str == "0" || str == "false");
	}
		break;
	case Type::DATA:
		break;
	case Type::DATE:
		break;
	case Type::VARIANTVECTOR:
		break;
	case Type::VARIANTMAP:
		break;
	case Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
    return false;
}

char Variant::toChar() const
{
    _assert(convert(Type::CHAR), "Can't convert to char.");
	switch (_type)
	{
	case Type::NONE:
		break;
	case Type::BOOLEAN: return static_cast<char>(_field.boolVal);
		break;
	case Type::CHAR: return _field.charVal;
		break;
	case Type::BYTE: return _field.byteVal;
		break;
	case Type::INTEGER: return _field.intVal;
		break;
	case Type::UINTEGER: return _field.unsignedVal;
		break;
	case Type::FLOAT: return static_cast<char>(_field.floatVal);
		break;
	case Type::DOUBLE: return static_cast<char>(_field.doubleVal);
		break;
	case Type::LONGLONG: return static_cast<char>(_field.longLongVal);
		break;
	case Type::ULONGLONG: return static_cast<char>(_field.unsignedLongLongVal);
		break;
	case Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<char>(atoi(str.c_str()));
	}
		break;
	case Type::DATA:
		break;
	case Type::DATE:
		break;
	case Type::VARIANTVECTOR:
		break;
	case Type::VARIANTMAP:
		break;
	case Type::VARIANTMAPINTKEY:
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
	_assert(convert(Type::INTEGER), "Can't convert to integer.");
	switch (_type)
	{
	case Type::NONE:
		break;
	case Type::BOOLEAN: return static_cast<int>(_field.boolVal);
		break;
	case Type::CHAR: return _field.charVal;
		break;
	case Type::BYTE: return _field.byteVal;
		break;
	case Type::INTEGER: return _field.intVal;
		break;
	case Type::UINTEGER: return _field.unsignedVal;
		break;
	case Type::FLOAT: return static_cast<int>(_field.floatVal);
		break;
	case Type::DOUBLE: return static_cast<int>(_field.doubleVal);
		break;
	case Type::LONGLONG: return static_cast<int>(_field.longLongVal);
		break;
	case Type::ULONGLONG: return static_cast<int>(_field.unsignedLongLongVal);
		break;
	case Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<int>(atoi(str.c_str()));
	}
		break;
	case Type::DATA:
		break;
	case Type::DATE:
		break;
	case Type::VARIANTVECTOR:
		break;
	case Type::VARIANTMAP:
		break;
	case Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return 0;
}

unsigned int Variant::toUInt() const
{
	_assert(convert(Type::UINTEGER), "Can't convert to unsigned integer.");
	return static_cast<unsigned int>(toInt());
}

float Variant::toFloat() const
{
	_assert(convert(Type::FLOAT), "Can't convert to float.");
	return static_cast<float>(toDouble());
}

double Variant::toDouble() const
{
	_assert(convert(Type::DOUBLE), "Can't convert to double.");
	switch (_type)
	{
	case Type::NONE:
		break;
	case Type::BOOLEAN: return static_cast<double>(_field.boolVal);
		break;
	case Type::CHAR: return _field.charVal;
		break;
	case Type::BYTE: return _field.byteVal;
		break;
	case Type::INTEGER: return _field.intVal;
		break;
	case Type::UINTEGER: return _field.unsignedVal;
		break;
	case Type::FLOAT: return static_cast<double>(_field.floatVal);
		break;
	case Type::DOUBLE: return static_cast<double>(_field.doubleVal);
		break;
	case Type::LONGLONG: return static_cast<double>(_field.longLongVal);
		break;
	case Type::ULONGLONG: return static_cast<double>(_field.unsignedLongLongVal);
		break;
	case Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<double>(atof(str.c_str()));
	}
		break;
	case Type::DATA:
		break;
	case Type::DATE:
		break;
	case Type::VARIANTVECTOR:
		break;
	case Type::VARIANTMAP:
		break;
	case Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return 0.0;
}

long long Variant::toLongLong() const
{
	_assert(convert(Type::LONGLONG), "Can't convert to long long.");
	switch (_type)
	{
	case Type::NONE:
		break;
	case Type::BOOLEAN: return static_cast<long long>(_field.boolVal);
		break;
	case Type::CHAR: return _field.charVal;
		break;
	case Type::BYTE: return _field.byteVal;
		break;
	case Type::INTEGER: return _field.intVal;
		break;
	case Type::UINTEGER: return _field.unsignedVal;
		break;
	case Type::FLOAT: return static_cast<long long>(_field.floatVal);
		break;
	case Type::DOUBLE: return static_cast<long long>(_field.doubleVal);
		break;
	case Type::LONGLONG: return static_cast<long long>(_field.longLongVal);
		break;
	case Type::ULONGLONG: return static_cast<long long>(_field.unsignedLongLongVal);
		break;
	case Type::STRING: {
		const string &str = *(const string *)_field.object;
		return static_cast<long long>(atoll(str.c_str()));
	}
		break;
	case Type::DATA:
		break;
	case Type::DATE:
		break;
	case Type::VARIANTVECTOR:
		break;
	case Type::VARIANTMAP:
		break;
	case Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return 0;
}

unsigned long long Variant::toULongLong() const
{
	_assert(convert(Type::ULONGLONG), "Can't convert to unsigned long long.");
	return static_cast<unsigned long long>(toLongLong());
}

string Variant::toString()
{
	_assert(convert(Type::STRING), "Can't convert to string");
	if (_type == Type::STRING) {
        if (!_field.object) {
            return string("(null)");
        }
		return *static_cast<string *>(_field.object);
	}
	stringstream ss;
	switch (_type)
	{
	case Type::NONE: ss << "(null)";
		break;
	case Type::BOOLEAN: ss << (_field.boolVal ? "true" : "false");
		break;
	case Type::CHAR: ss << _field.charVal;
		break;
	case Type::BYTE: ss << _field.byteVal;
		break;
	case Type::INTEGER: ss << _field.intVal;
		break;
	case Type::UINTEGER: ss << _field.unsignedVal;
		break;
	case Type::FLOAT: ss << _field.floatVal;
		break;
	case Type::DOUBLE: ss << _field.doubleVal;
		break;
	case Type::LONGLONG: ss << _field.longLongVal;
		break;
	case Type::ULONGLONG: ss << _field.unsignedLongLongVal;
		break;
	case Type::STRING:
		break;
	case Type::DATA:
		break;
    case Type::DATE: ss << *static_cast<Date *>(_field.object);
		break;
	case Type::VARIANTVECTOR:
		break;
	case Type::VARIANTMAP:
		break;
	case Type::VARIANTMAPINTKEY:
		break;
	default:
		break;
	}
	return ss.str();
}

const Date& Variant::toDate() const
{
	_assert(convert(Type::DATE), "Can't convert to Date.");
	return *static_cast<const Date *>(_field.object);
}

string& Variant::toString() const
{
	_assert(_type == Type::STRING, "Can't convert to string.");
	return *static_cast<string *>(_field.object);
}

VariantVector & Variant::toVariantVector()
{
	_assert(convert(Type::VARIANTVECTOR), "Can't convert to VariantVector(std::vector<Variant>).");
	return *static_cast<VariantVector *>(_field.object);
}

const VariantVector & Variant::toVariantVector() const
{
    return const_cast<Variant *>(this)->toVariantVector();
}

VariantMap & Variant::toVariantMap()
{
	_assert(convert(Type::VARIANTMAP), "Can't convert to VariantMap(std::unordered_map<std::string, Variant>).");
	return *static_cast<VariantMap *>(_field.object);
}

const VariantMap & Variant::toVariantMap() const
{
    return const_cast<Variant *>(this)->toVariantMap();
}

VariantMapIntKey & Variant::toVariantMapIntKey()
{
	_assert(convert(Type::VARIANTMAPINTKEY), "Can't convert to VariantMapIntKey(std::unordered_map<int, Variant>).");
	return *static_cast<VariantMapIntKey *>(_field.object);
}

const VariantMapIntKey & Variant::toVariantMapIntKey() const
{
	return const_cast<Variant *>(this)->toVariantMapIntKey();
}

VariantData & Variant::toVariantData()
{
	_assert(convert(Type::DATA) ,"Can't convert to VariantData(std::vector<unsigned char>)");
	return *static_cast<VariantData *>(_field.object);
}

const VariantData & Variant::toVariantData() const
{
	return const_cast<Variant *>(this)->toVariantData();
}

Variant & Variant::operator=(const Variant & other)
{
	if (this != &other) {
		reset(other._type);
		switch (_type) {
		case Type::NONE:
			break;
		case Type::BOOLEAN:
		case Type::CHAR:
		case Type::BYTE:
		case Type::INTEGER:
		case Type::UINTEGER:
		case Type::FLOAT:
		case Type::DOUBLE:
		case Type::LONGLONG:
		case Type::ULONGLONG:
			_field.unsignedLongLongVal = other._field.unsignedLongLongVal;
			break;
		case Type::STRING:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) string(*static_cast<string *>(other._field.object));
			} else {
				auto &v = *static_cast<string *>(_field.object);
				v = *static_cast<string *>(other._field.object);
			}
			break;
		case Type::DATA:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantData (*static_cast<VariantData *>(other._field.object));
			} else {
				auto &v = *static_cast<VariantData *>(_field.object);
				v = *static_cast<VariantData *>(other._field.object);
			}
			break;
		case Type::DATE:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantData(*static_cast<VariantData *>(other._field.object));
			}
			else {
				auto &v = *static_cast<VariantData *>(_field.object);
				v = *static_cast<VariantData *>(other._field.object);
			}
			break;
		case Type::VARIANTVECTOR:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantVector(*static_cast<VariantVector *>(other._field.object));
			}
			else {
				auto &v = *static_cast<VariantVector *>(_field.object);
				v = *static_cast<VariantVector *>(other._field.object);
			}
			break;
		case Type::VARIANTMAP:
			if (_field.object == nullptr) {
				_field.object = new (nothrow) VariantMap(*static_cast<VariantMap *>(other._field.object));
			}
			else {
				auto &v = *static_cast<VariantMap *>(_field.object);
				v = *static_cast<VariantMap *>(other._field.object);
			}
			break;
		case Type::VARIANTMAPINTKEY:
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

Variant & Variant::operator=(long long v)
{
    reset(Type::LONGLONG);
    _field.longLongVal = v;
    return *this;
}

Variant & Variant::operator=(unsigned long long v)
{
    reset(Type::ULONGLONG);
    _field.longLongVal = v;
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

//Variant & Variant::operator=(shared_ptr<Date> &&v)
//{
//    clear();
//    _type = Type::DATE;
//    v.reset();
//    _field.object = 0;
//    return *this;
//}

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
	case Type::NONE:
		break;
	case Type::BOOLEAN:
	case Type::CHAR:
	case Type::BYTE:
	case Type::INTEGER:
	case Type::UINTEGER:
		return _field.unsignedVal == v._field.unsignedVal;
	case Type::FLOAT:
		return std::abs(v._field.floatVal - _field.floatVal) <= FLT_EPSILON;
	case Type::DOUBLE:
		return std::abs(v._field.doubleVal - _field.doubleVal) <= FLT_EPSILON;
	case Type::LONGLONG:
	case Type::ULONGLONG:
		return _field.unsignedLongLongVal == v._field.unsignedLongLongVal;
		break;
	case Type::STRING:
		return *static_cast<string *>(_field.object) == *static_cast<string *>(v._field.object);
		break;
	case Type::DATA:
		return *static_cast<VariantData *>(_field.object) == *static_cast<VariantData *>(v._field.object);
		break;
	case Type::DATE:
		return *static_cast<Date *>(_field.object) == *static_cast<Date *>(v._field.object);
		break;
	case Type::VARIANTVECTOR:
		return *static_cast<VariantVector *>(_field.object) == *static_cast<VariantVector *>(v._field.object);
		break;
	case Type::VARIANTMAP: {
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
	case Type::VARIANTMAPINTKEY: {
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
    _assert(_type <= Type::ULONGLONG, "Memory Error!");
    parameterAssert(type <= Type::VARIANTMAPINTKEY);
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
