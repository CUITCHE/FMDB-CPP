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

typedef std::vector<Variant> VariantVector;
typedef std::unordered_map<std::string, Variant> VariantMap;
typedef std::unordered_map<int, Variant> VariantMapIntKey;

class Variant
{
public:
    Variant();

    explicit Variant(bool v);
    explicit Variant(char v);
    explicit Variant(unsigned char v);
    explicit Variant(int v);
    explicit Variant(unsigned int v);
    explicit Variant(float v);
    explicit Variant(double v);
    explicit Variant(long long v);
    explicit Variant(unsigned long long v);

    explicit Variant(const vector<unsigned char> &v);
    explicit Variant(const vector<unsigned char> &&v);

    explicit Variant(const Date &v);

    explicit Variant(const char *v);
    explicit Variant(const string &v);
    explicit Variant(string &&v);

    explicit Variant(const VariantVector &v);
    explicit Variant(VariantVector &&v);

    explicit Variant(const VariantMap &v);
    explicit Variant(VariantMap &&v);

    explicit Variant(const VariantMapIntKey &v);
    explicit Variant(VariantMapIntKey &&v);

    ~Variant();

    enum class Type {
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

    /**
     * Checks if the Value is null.
     * @return True if the Value is null, false if not.
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
    string toString() const;
    
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
    }_field{ 0 };
    Type _type;
};

FMDB_END

#endif /* Variant_hpp */
