//
//  Variant.hpp
//  FMDB-CPP
//
//  Also see cocos2d-x's Value.
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#ifndef Variant_hpp
#define Variant_hpp

#include "FMDBDefs.h"
#include <unordered_map>

FMDB_BEGIN

class Variant;
class Date;

using VariantVector		= std::vector<Variant>;
using VariantMap		= std::unordered_map<std::string, Variant>;
using VariantMapIntKey	= std::unordered_map<int, Variant>;
using VariantData		= std::vector<unsigned char>;

class Variant
{
public:
	static const Variant null;
    Variant();

    Variant(bool v);
    Variant(char v);
    Variant(unsigned char v);
    Variant(int v);
    Variant(unsigned int v);
    Variant(float v);
    Variant(double v);
    Variant(long long v);
    Variant(unsigned long long v);

    Variant(const VariantData &v);
    Variant(VariantData &&v);

    Variant(const Date &v);

    Variant(const char *v);
    Variant(const string &v);
    Variant(string &&v);

    Variant(const VariantVector &v);
    Variant(VariantVector &&v);

    Variant(const VariantMap &v);
    Variant(VariantMap &&v);

    Variant(const VariantMapIntKey &v);
    Variant(VariantMapIntKey &&v);

	Variant(const Variant &other);
	Variant(Variant &&other);

    Variant(std::nullptr_t);

    ~Variant();

    enum class Type : unsigned long long {
        NONE = 0,
        BOOLEAN,
        CHAR,
        BYTE,
        INTEGER,
        UINTEGER,
        FLOAT,
        DOUBLE,
        LONGLONG,
        ULONGLONG,
        STRING,
        DATA,
        DATE,
        VARIANTVECTOR,
        VARIANTMAP,
        VARIANTMAPINTKEY
    };

    /**
     Return true if Varaint can convert with toType, otherwise false.
     
     @see Type

     @param toType Transformed type.
     @return Return true if success, otherwise false.
     */
    bool convert(Type toType) const;

    bool isTypeOf(Type type) const { return type == _type; }

    /**
     * Checks if the Variant is null.
     * @return True if the Variant is null, false if not.
     */
	bool isNull() const { return _type == Type::NONE; }

    /** Gets the value type. */
    Type getType() const { return _type; }

    /** Gets the description of the class. */
    std::string description() const;

    bool toBool() const;
    char toChar() const;
    unsigned char toByte() const;
    int toInt() const;
    unsigned int toUInt() const;
    float toFloat() const;
    double toDouble() const;
    long long toLongLong() const;
    unsigned long long toULongLong() const;
    string toString();
	const Date& toDate() const;
    
	/** Return stored string object if type is STRING, otherwise will trigger error. */
	string& toString() const;

	VariantVector& toVariantVector();
	const VariantVector& toVariantVector() const;

	VariantMap& toVariantMap();
	const VariantMap& toVariantMap() const;

	VariantMapIntKey& toVariantMapIntKey();
	const VariantMapIntKey& toVariantMapIntKey() const;

	VariantData& toVariantData();
	const VariantData& toVariantData() const;

	/* operators */
	/** Assignment operator, assign from Variant to Variant. */
	Variant& operator= (const Variant& other);
	/** Assignment operator, assign from Variant to Variant. It will use std::move internally. */
	Variant& operator= (Variant&& other);

	/** Assignment operator, assign from unsigned char to Variant. */
	Variant& operator= (unsigned char v);
	/** Assignment operator, assign from integer to Variant. */
	Variant& operator= (int v);
	/** Assignment operator, assign from integer to Variant. */
	Variant& operator= (unsigned int v);
	/** Assignment operator, assign from float to Variant. */
	Variant& operator= (float v);
	/** Assignment operator, assign from double to Variant. */
	Variant& operator= (double v);
	/** Assignment operator, assign from bool to Variant. */
	Variant& operator= (bool v);
    /** Assignment operator, assign from long long to Variant. */
    Variant& operator= (long long v);
    /** Assignment operator, assign from unsigned long long to Variant. */
    Variant& operator= (unsigned long long v);
	/** Assignment operator, assign from char* to Variant. */
	Variant& operator= (const char* v);
	/** Assignment operator, assign from string to Variant. */
	Variant& operator= (const std::string& v);
	/** Assignment operator, assign from Date to Variant. */
	Variant& operator= (const Date& v);
//    Variant& operator= (std::shared_ptr<Date> &&rhs);

	/** Assignment operator, assign from VariantVector to Variant. */
	Variant& operator= (const VariantVector& v);
	/** Assignment operator, assign from VariantVector to Variant. */
	Variant& operator= (VariantVector&& v);

	/** Assignment operator, assign from VariantMap to Variant. */
	Variant& operator= (const VariantMap& v);
	/** Assignment operator, assign from VariantMap to Variant. It will use std::move internally. */
	Variant& operator= (VariantMap&& v);

	/** Assignment operator, assign from VariantMapIntKey to Variant. */
	Variant& operator= (const VariantMapIntKey& v);
	/** Assignment operator, assign from VariantMapIntKey to Variant. It will use std::move internally. */
	Variant& operator= (VariantMapIntKey&& v);
	/** Assignment operator, assign from VariantData to Variant. */
	Variant& operator=(const VariantData &v);
	/** Assignment operator, assign from VariantData to Variant. It will use std::move internally. */
	Variant& operator=(VariantData &&v);

	/** != operator overloading */
	bool operator!= (const Variant& v);
	/** != operator overloading */
	bool operator!= (const Variant& v) const;
	/** == operator overloading */
	bool operator== (const Variant& v);
	/** == operator overloading */
	bool operator== (const Variant& v) const;

	operator bool() const { return !isNull(); }
private:
    void clear();
    void reset(Type type);
    union {
        bool boolVal;
        char charVal;
        unsigned char byteVal;
        int intVal;
        unsigned int unsignedVal;
        float floatVal;
        double doubleVal;
        long long longLongVal;
        unsigned long long unsignedLongLongVal;
        void *object; // Data, Date, String, etc.
//        std::unique_ptr<void> *object;
    }_field{ 0 };
    Type _type;
};

FMDB_END

#endif /* Variant_hpp */
