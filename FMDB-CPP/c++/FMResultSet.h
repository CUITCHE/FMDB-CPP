//
//  FMResultSet.h
//  fmdb
//
//  Created by hejunqiu on 2017/2/9.
//
//

#ifndef FMResultSet_hpp
#define FMResultSet_hpp

#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::vector;
using std::unordered_map;

#if defined(NS_CC)
#include "CCRef.h"

class CCValue;

#endif

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
    const string& columnNameForIndex(int columnIndex) const;

    int intForColumn(const string &columnName) const;
    int intForColumnIndex(int colunmIndex) const;

    long longForColumn(const string &coumnName) const;
    long longForColumnIndex(int columnIndex) const;

    long long longLongForColumn(const string &coumnName) const;
    long long longLongForColumnIndex(int columnIndex) const;

    bool boolForColumn(const string &coumnName) const;
    bool boolForColumnIndex(int columnIndex) const;

    double doubleForColumn(const string &coumnName) const;
    double doubleForColumnIndex(int columnIndex) const;

    string stringForColumn(const string &coumnName) const;
    string stringForColumnIndex(int columnIndex) const;

    vector<unsigned char> dataForColumn(const string &coumnName) const;
    vector<unsigned char> dataForColumnIndex(int columnIndex) const;

    const unsigned char * UTF8StringForColumn(const string &coumnName) const;
    const unsigned char * UTF8StringForColumnIndex(int columnIndex) const;
#if defined(NS_CC)
    // 以后需要改成自己写的Value class
    CCValue valueForColumn(const string &columnName) const;
    CCValue valueForColumnIndex(int colunmIndex) const;
#endif

    bool columnIndexIsNull(int columnIndex) const;
    bool columnIsNull(const string &columnName) const;

	void setStatement(FMStatement *stmt) { _statement = stmt; }
	void setParentDB(FMDatabase *db) { _parentDB = db; }

	unordered_map<string, int>& columnNameToIndexMap();
private:
    FMDatabase *_parentDB;
    FMStatement *_statement;
    string _query;
    unordered_map<string, int> _columnNameToIndexMap;
};

#endif /* FMResultSet_hpp */
