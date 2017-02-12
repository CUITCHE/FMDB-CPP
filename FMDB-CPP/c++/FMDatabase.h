//
//  FMDatabase.h
//  fmdb
//
//  Created by hejunqiu on 2017/2/9.
//
//

#ifndef FMDatabase_hpp
#define FMDatabase_hpp

#include <unordered_map>
#include <unordered_set>
#include "FMDBDefs.h"
#include "Variant.hpp"

using std::unordered_map;
using std::unordered_set;
using std::unique_ptr;

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

FMDB_BEGIN

class FMStatement;
class FMResultSet;
/*class Variant;*/

class FMDatabase
{
public:
    using StatemenCacheType = unique_ptr<unordered_map<string, unordered_set<FMStatement *>>>;

    static FMDatabase databaseWithPath(const string &path);
    FMDatabase(const string &filepath);
    ~FMDatabase();

    bool traceExecution() const { return _traceExecution; }
    bool checkedOut() const { return _checkedOut; }
    bool crashOnErrors() const { return _crashOnErrors; }
    bool logsErrors() const { return _logsErrors; }
    const StatemenCacheType &cachedStatements() const { return _cachedStatements; }

    bool open();
    bool openWithFlags(int flags);
    bool openWithFlags(int flags, const string &vfs);

    bool close();

    bool isGoodConnection() const;

    long long lastInsertRowId();
    int changes();

    void resultSetDidClose(FMResultSet *resultSet);

    /* Transactions */
    bool beginTransaction();
    bool beginDeferredTransaction();

    bool commit();
    bool rollback();
    bool inTransaction();

    /* Cached statements and result sets */
    void clearCachedStatements();
    void closeOpenResultSets();
    bool hasOpenResultSets();
    bool shouldCacheStatements();
    void setShouldCacheStatements(bool value);

    bool interrupt();

    /* Encryption */
    bool setKey(const string &key);
    bool setKey(const vector<char> &key);

    bool reKey(const string &key);
    bool reKey(const vector<char> &key);

    /* General inquiry methods */
    const string &databasePath() const { return _databasePath; };
    sqlite3 *sqliteHandle() const { return _db; };

    /* Retrieving error codes */
    string lastErrorMessage() const;
    int lastErrorCode() const;
    int lastExtendedErrorCode() const;
    bool hadError() const;

    void setMaxBusyRetryTimeInterval(TimeInterval timeoutInSeconds);
    double maxBusyRetryTimeInterval() const { return _maxBusyRetryTimeInterval.count(); };

	/** execute sql templates */
	template<typename... Args>
	FMResultSet *executeQuery(const string &sql, Args... args);
private:
	bool executeQueryPrepareAndCheck(const string &sql, sqlite3_stmt *&pStmt, FMStatement *&statement);
	bool executeQueryParametersCheck(int inputParametersCount, sqlite3_stmt *pStmt);
	FMResultSet *executeQueryImpl(const string &sql, FMStatement *statement, sqlite3_stmt *pStmt);
	template<int paramN, typename... Args, typename T>
	void bindObjects(sqlite3_stmt *inStmt, const T &v, Args... args);
	template<int paramN>
	void bindObjects(sqlite3_stmt *inStmt);
private:
    const char *sqlitePath() const;
    friend int FMDBDatabaseBusyHandler(void *f, int count);

    FMStatement *cachedStatementForQuery(const string &query);
    void setCachedStatement(FMStatement *statement, const string &query);

    void warnInUse() const;
    bool databaseExists() const;

	void bindObject(const Variant &obj, int toColumn, sqlite3_stmt *inStmt);
private:
    uint32_t _logsErrors : 1;
    uint32_t _crashOnErrors : 1;
    uint32_t _traceExecution : 1;
    uint32_t _checkedOut : 1;
    uint32_t _shouldCacheStatements : 1;
    volatile uint32_t _isExecutingStatement : 1;
    uint32_t _inTransaction : 1;
#if __LP64__
    uint32_t reserve;
#endif
    sqlite3 *_db = nullptr;
    TimeInterval _maxBusyRetryTimeInterval = TimeInterval(2); // 2 seconds
    TimeInterval _startBusyRetryTime;
    StatemenCacheType _cachedStatements;
    unique_ptr<unordered_set<FMResultSet *>> _openResultSets;
    unique_ptr<unordered_set<FMStatement *>> _openFunctions;
    const string _databasePath;
};

template<typename ...Args>
inline FMResultSet * FMDatabase::executeQuery(const string &sql, Args... args)
{
	sqlite3_stmt *pStmt		= 0;
	FMStatement *statement	= 0;
	if (!executeQueryPrepareAndCheck(sql, pStmt, statement)) { // Sqlite environment check
		return nullptr;
	}
	if (!executeQueryParametersCheck(sizeof...(args), pStmt)) { // Parameters count check
		return nullptr;
	}
	bindObjects<0>(pStmt, std::forward<Args>(args)...);
	return executeQueryImpl(sql, statement, pStmt);
}

template<int paramN, typename ...Args, typename T>
inline void FMDatabase::bindObjects(sqlite3_stmt * inStmt, const T & v, Args... args)
{
	bindObject(Variant(v), paramN, inStmt);
	bindObjects<paramN + 1>(inStmt, std::forward<Args>(args)...);
}

template<int paramN>
inline void FMDatabase::bindObjects(sqlite3_stmt * inStmt)
{}

FMDB_END

#endif /* FMDatabase_hpp */