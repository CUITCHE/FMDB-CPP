//
//  FMResultSet.cpp
//  fmdb
//
//  Created by hejunqiu on 2017/2/9.
//
//

#include "FMResultSet.h"
#include "FMStatement.hpp"
#include "FMDatabase.h"
#include <algorithm>

using namespace std;

#if FMDB_SQLITE_STANDALONE
#include <sqlite3/sqlite3.h>
#else
#include <sqlite3.h>
#endif

FMDB_BEGIN

FMResultSet *FMResultSet::resultSet(FMStatement *statement, FMDatabase *parentDatabase)
{
	FMResultSet *rs = new FMResultSet();
	rs->setStatement(statement);
	rs->setParentDB(parentDatabase);
	statement->setInUse(true);
	return rs;
}

void FMResultSet::close()
{
	_statement->reset();
	_statement = nullptr;

	_parentDB->resultSetDidClose(this);
	_parentDB = nullptr;
}

bool FMResultSet::next()
{
	return nextWithError(nullptr);
}

bool FMResultSet::nextWithError(void **error)
{
	int rc = sqlite3_step(_statement->getStatement());

	if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc) {
		fprintf(stderr, "%s:%d Database busy(%s)", __FUNCTION__, __LINE__, _parentDB->databasePath().c_str());
		fprintf(stderr, "Database busy");
// 		if (outErr) {
// 			*outErr = [_parentDB lastError];
// 		}
	}
	else if (SQLITE_DONE == rc || SQLITE_ROW == rc) {
		// all is well, let's return.
	} else if (SQLITE_ERROR == rc) {
		fprintf(stderr, "Error calling sqlite3_step(%d: %s) rs", rc, sqlite3_errmsg(_parentDB->sqliteHandle()));
// 		if (outErr) {
// 			*outErr = [_parentDB lastError];
// 		}
	} else if (SQLITE_MISUSE == rc) {
		// uh oh.
		fprintf(stderr, "Error calling sqlite3_step(%d: %s) rs", rc, sqlite3_errmsg(_parentDB->sqliteHandle()));
// 		if (outErr) {
// 			if (_parentDB) {
// 				*outErr = [_parentDB lastError];
// 			} else {
// 				// If 'next' or 'nextWithError' is called after the result set is closed,
// 				// we need to return the appropriate error.
// 				NSDictionary* errorMessage = [NSDictionary dictionaryWithObject : @"parentDB does not exist" forKey:NSLocalizedDescriptionKey];
// 					*outErr = [NSError errorWithDomain : @"FMDatabase" code : SQLITE_MISUSE userInfo : errorMessage];
// 			}
// 		}
	} else {
		// wtf?
		fprintf(stderr, "Unknown error calling sqlite3_step(%d: %s) rs", rc, sqlite3_errmsg(_parentDB->sqliteHandle()));
// 		if (outErr) {
// 			*outErr = [_parentDB lastError];
// 		}
	}
	if (rc != SQLITE_ROW) {
		close();
	}
	return (rc == SQLITE_ROW);
}

bool FMResultSet::hasAnotherRow() const
{
	return sqlite3_errcode(_parentDB->sqliteHandle()) == SQLITE_ROW;
}

int FMResultSet::columnCount() const
{
	return sqlite3_column_count(_statement->getStatement());
}

int FMResultSet::columnIndexForName(const string &columnName) const
{
	string str(columnName);
	transform(str.begin(), str.end(), str.begin(), tolower);
	auto iter = columnNameToIndexMap().find(str);
	if (iter != columnNameToIndexMap().end()) {
		return iter->second;
	} else {
		fprintf(stderr, "Warning: I could not find the column named '%s'.", columnName.c_str());
		return -1;
	}
}

string FMResultSet::columnNameForIndex(int columnIndex) const
{
    return string(sqlite3_column_name(_statement->getStatement(), columnIndex));
}

int FMResultSet::intForColumnIndex(int colunmIndex) const
{
    return sqlite3_column_int(_statement->getStatement(), colunmIndex);
}

long FMResultSet::longForColumnIndex(int columnIndex) const
{
    return
#if __LP64__
    sqlite3_column_int64(_statement->getStatement(), columnIndex);
#else
    sqlite3_column_int(_statement->getStatement(), columnIndex);
#endif
}

long long FMResultSet::longLongForColumnIndex(int columnIndex) const
{
    return sqlite3_column_int64(_statement->getStatement(), columnIndex);
}

double FMResultSet::doubleForColumnIndex(int columnIndex) const
{
    return sqlite3_column_double(_statement->getStatement(), columnIndex);
}

bool FMResultSet::boolForColumnIndex(int columnIndex) const
{
    return !!sqlite3_column_int(_statement->getStatement(), columnIndex);
}

String FMResultSet::stringForColumnIndex(int columnIndex) const
{
    if (sqlite3_column_type(_statement->getStatement(), columnIndex) == SQLITE_NULL || (columnIndex < 0)) {
        return String();
    }
    const char *c = (const char *)sqlite3_column_text(_statement->getStatement(), columnIndex);
    if (!c) {
        return String();
    }
    return make_shared<String::element_type>(c);
}

Data FMResultSet::dataForColumnIndex(int columnIndex) const
{
    if (sqlite3_column_type(_statement->getStatement(), columnIndex) == SQLITE_NULL || (columnIndex < 0)) {
        return Data();
    }

    const char *dataBuffer = (const char *)sqlite3_column_blob(_statement->getStatement(), columnIndex);
    int dataSize = sqlite3_column_bytes(_statement->getStatement(), columnIndex);

    if (dataBuffer == NULL) {
        return Data();
    }

    return make_shared<Data::element_type>(dataBuffer, dataBuffer+dataSize);
}

const unordered_map<string, int>& FMResultSet::columnNameToIndexMap() const
{
	if (_columnNameToIndexMap.size() == 0) {
		int columnCount = sqlite3_column_count(_statement->getStatement());
        auto remove_cv_this = (std::remove_cv<std::remove_pointer<decltype(this)>::type>::type *)this;
        remove_cv_this->_columnNameToIndexMap.reserve(columnCount);
		for (int i = 0;i < columnCount;++i) {
			string str(sqlite3_column_name(_statement->getStatement(), i));
			transform(str.begin(), str.end(), str.begin(), tolower);
			remove_cv_this->_columnNameToIndexMap.emplace(str, i);
		}
	}
	return _columnNameToIndexMap;
}

FMDB_END
