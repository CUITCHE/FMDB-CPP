//
//  FMResultSet.h
//  fmdb
//
//  Created by hejunqiu on 2017/2/9.
//
//

#ifndef FMResultSet_hpp
#define FMResultSet_hpp

#include <unordered_map>
#include "FMDBDefs.h"
#include "Date.hpp"

using std::unordered_map;

#if defined(NS_CC)
#include "CCRef.h"

class CCValue;

#endif

FMDB_BEGIN

class FMDatabase;
class FMStatement;

class FMResultSet
{
public:
    static FMResultSet *resultSet(FMStatement *statement, FMDatabase *parentDatabase);
	FMResultSet() { }
	~FMResultSet() { close(); }

    void close();

    bool next();
    bool nextWithError(void **error);
    bool hasAnotherRow() const;

    int columnCount() const;

    int columnIndexForName(const string &columnName) const;
    string columnNameForIndex(int columnIndex) const;

    int intForColumn(const string &columnName) const { return intForColumnIndex(columnIndexForName(columnName)); }
    int intForColumnIndex(int colunmIndex) const;

    long longForColumn(const string &columnName) const { return longForColumnIndex(columnIndexForName(columnName)); }
    long longForColumnIndex(int columnIndex) const;

    long long longLongForColumn(const string &columnName) const { return longLongForColumnIndex(columnIndexForName(columnName)); }
    long long longLongForColumnIndex(int columnIndex) const;

    bool boolForColumn(const string &columnName) const { return boolForColumnIndex(columnIndexForName(columnName)); }
    bool boolForColumnIndex(int columnIndex) const;

    double doubleForColumn(const string &columnName) const { return doubleForColumnIndex(columnIndexForName(columnName)); }
    double doubleForColumnIndex(int columnIndex) const;

    String stringForColumn(const string &columnName) const { return stringForColumnIndex(columnIndexForName(columnName)); }
    String stringForColumnIndex(int columnIndex) const;

    Data dataForColumn(const string &columnName) const { return dataForColumnIndex(columnIndexForName(columnName)); }
    Data dataForColumnIndex(int columnIndex) const;

    shared_ptr<Date> dateForColumn(const string &columnName) const { return dateForColumnIndex(columnIndexForName(columnName)); }
    shared_ptr<Date> dateForColumnIndex(int columnIndex) const;

    const unsigned char * UTF8StringForColumn(const string &columnName) const { return UTF8StringForColumnIndex(columnIndexForName(columnName)); }
    const unsigned char * UTF8StringForColumnIndex(int columnIndex) const;
#if defined(NS_CC)
    // 以后需要改成自己写的Value class
    CCValue valueForColumn(const string &columnName) const;
    CCValue valueForColumnIndex(int colunmIndex) const;
#endif

    bool columnIsNull(const string &columnName) const { return columnIndexIsNull(columnIndexForName(columnName)); }
    bool columnIndexIsNull(int columnIndex) const;

	void setStatement(FMStatement *stmt) { _statement = stmt; }
	void setParentDB(FMDatabase *db) { _parentDB = db; }
	void setQuery(const string &query) { _query = query; }

	const unordered_map<string, int>& columnNameToIndexMap() const;
private:
    FMDatabase *_parentDB;
    FMStatement *_statement;
    string _query;
    unordered_map<string, int> _columnNameToIndexMap;
};

FMDB_END

#endif /* FMResultSet_hpp */
