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
#include <functional>
#include "FMDBDefs.h"
#include "Variant.hpp"
#include "Error.hpp"
#include "FMResultSet.h"

using std::unordered_map;
using std::unordered_set;
using std::unique_ptr;
using std::function;

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

FMDB_BEGIN

class FMStatement;
class FMResultSet;
/*class Variant;*/

extern const string FMDatabaseNullFilePath;

class FMDatabase
{
public:
    using StatemenCacheType = unique_ptr<unordered_map<string, vector<shared_ptr<FMStatement>>>>;
    using FMDBExecuteStatementsCallbackBlock = function<int(unordered_map<string, Variant> &result)>;

    FMDatabase(const string &filepath = FMDatabaseNullFilePath);
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

    static string sqliteLibVersion();
    static bool isSQLiteThreadSafe();

    bool isGoodConnection();

    long long lastInsertRowId();
    int changes();

    void resultSetDidClose(FMResultSet *resultSet);

    /* Transactions */
    bool beginTransaction();
    bool beginDeferredTransaction();

    bool commit();
    bool rollback();
    bool inTransaction();

    bool startSavePointWithName(const string &name, Error *error = nullptr);
    bool releaseSavePointWithName(const string &name, Error *error = nullptr);
    bool rollbackToSavePointWithName(const string &name, Error *error = nullptr);
    bool inSavePoint(const function<void(bool *rollback)> &block);

    /* Cached statements and result sets */
    void clearCachedStatements();
    void closeOpenResultSets();
    bool hasOpenResultSets();
	bool shouldCacheStatements() const { return _shouldCacheStatements; }
	void setShouldCacheStatements(bool value) { _shouldCacheStatements = value; }

    bool interrupt();

    /* Encryption */
    bool setKey(const string &key);
    bool setKey(const vector<char> &key);

    bool reKey(const string &key);
    bool reKey(const vector<char> &key);

    /* General inquiry methods */
    const string &databasePath() const { return *_databasePath; };
    sqlite3 *sqliteHandle() const { return _db; };

    /* Retrieving error codes */
    string lastErrorMessage() const;
    Error lastError() const;
    int lastErrorCode() const;
    int lastExtendedErrorCode() const;
    bool hadError() const;

    void setMaxBusyRetryTimeInterval(TimeInterval timeoutInSeconds);
    double maxBusyRetryTimeInterval() const { return _maxBusyRetryTimeInterval.count(); };

	/** execute sql templates */

	/**
	 Execute select statement

	 @param sql The SQL to be performed, with optional `?` placeholders.
	 @param args Variable length packet.
	 @return A `<FMResultSet>` for the result set upon success; `nullptr` upon failure. If failed, you can call `<lastError>`, `<lastErrorCode>`, or `<lastErrorMessage>` for diagnostic information regarding the failure.
     
     @note DON'T release the return value.
	 */
	template<typename... Args>
	weak_ptr<FMResultSet> executeQuery(const string &sql, Args... args);

	/**
	 Execute single update statement

	 @param sql The SQL to be performed, with optional `?` placeholders.
	 @param args Variable length packet.
	 @return `YES` upon success; `NO` upon failure. If failed, you can call `<lastError>`, `<lastErrorCode>`, or `<lastErrorMessage>` for diagnostic information regarding the failure.
	 */
	template<typename... Args>
	bool executeUpdate(const string &sql, Args... args);


    /**
     Execute multiple SQL statements with callback handler or not.
     
      This executes a series of SQL statements that are combined in a single string (e.g. the SQL generated by the `sqlite3` command line `.dump` command). This accepts no value parameters, but rather simply expects a single string with multiple SQL statements, each terminated with a semicolon. This uses `sqlite3_exec`.

     @param sql The SQL to be performed.
     @param block A block that will be called for any result sets returned by any SQL statements.

     @note if you supply this block, it must return integer value, zero upon success (this would be a good opportunity to use SQLITE_OK), non-zero value upon failure (which will stop the bulk execution of the SQL).  If a statement returns values, the block will be called with the results from the query in NSDictionary *resultsDictionary. This may be `nullptr`(by default) if you don't care to receive any results.
     @return `YES` upon success; `NO` upon failure. If failed, you can call `<lastError>`, `<lastErrorCode>`, or `<lastErrorMessage>` for diagnostic information regarding the failure.
     */
    bool executeStatements(const string &sql, const FMDBExecuteStatementsCallbackBlock &block);
    bool executeStatements(const string &sql);

    /** callback function */
    void makeFunctionNamed(const string &name, int maximumArgument, const function<void(void *context, int argc, void **argv)> &block);

    /** convenience methods */
    template<typename... Args>
    int intForQuery(const string &sql, Args... args);

    template<typename... Args>
    long longForQuery(const string &sql, Args... args);

    template<typename... Args>
    long long longLongForQuery(const string &sql, Args... args);

    template<typename... Args>
    bool boolForQuery(const string &sql, Args... args);

    template<typename... Args>
    double doubleForQuery(const string &sql, Args... args);

    template<typename... Args>
    String stringForQuery(const string &sql, Args... args);

    template<typename... Args>
    Data dataForQuery(const string &sql, Args... args);

    template<typename... Args>
    shared_ptr<Date> dateForQuery(const string &sql, Args... args);

    bool tableExists(const string &tableName);

    FMResultSet *getSchema();
    FMResultSet *getTableSchema();

    bool columnExistsInTable(const string &tableName, const string &columnName);

    bool validateSQL(const string &sql, Error *error);

    uint32_t applicationID() const;
    void setApplicationID(uint32_t appID);

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
    string applicationIDString() const;
    void setApplicationIDString(const string &aString);
#endif
    uint32_t userVersion() const;
    void setUserVersion(uint32_t version);
private:
	bool executeQueryPrepareAndCheck(const string &sql, sqlite3_stmt *&pStmt, shared_ptr<FMStatement> &statement);
	bool executeQueryParametersCheck(int inputParametersCount, sqlite3_stmt *pStmt);

	weak_ptr<FMResultSet> executeQueryImpl(const string &sql, shared_ptr<FMStatement> &statement, sqlite3_stmt *pStmt);

	template<int paramN, typename... Args, typename T>
	void bindObjects(sqlite3_stmt *inStmt, const T &v, Args... args);

	template<int paramN>
	void bindObjects(sqlite3_stmt *inStmt);

	bool executeUpdateImpl(const string &sql, shared_ptr<FMStatement> &statement, sqlite3_stmt *pStmt);
private:
    const char *sqlitePath() const;
    friend int FMDBDatabaseBusyHandler(void *f, int count);

    shared_ptr<FMStatement> cachedStatementForQuery(const string &query);
    void setCachedStatement(shared_ptr<FMStatement> &statement, const string &query);

    void warnInUse() const;
    bool databaseExists() const;

	void bindObject(const Variant &obj, int toColumn, sqlite3_stmt *inStmt);
private:
    uint32_t _logsErrors : 1;
    uint32_t _crashOnErrors : 1;
    uint32_t _traceExecution : 1;
    uint32_t _checkedOut : 1;
	volatile uint32_t _shouldCacheStatements : 1;
    volatile uint32_t _isExecutingStatement : 1;
	volatile uint32_t _inTransaction : 1;
#if __PL64__
    uint32_t reserve;
#endif
    sqlite3 *_db = nullptr;
    TimeInterval _maxBusyRetryTimeInterval = TimeInterval(2); // 2 seconds
    TimeInterval _startBusyRetryTime;
    StatemenCacheType _cachedStatements;
    unique_ptr<vector<shared_ptr<FMResultSet>>> _openResultSets;
    unique_ptr<string> _databasePath;
};

template<typename ...Args>
inline weak_ptr<FMResultSet> FMDatabase::executeQuery(const string &sql, Args... args)
{
	sqlite3_stmt *pStmt		= 0;
	shared_ptr<FMStatement> statement;
	if (!executeQueryPrepareAndCheck(sql, pStmt, statement)) { // Sqlite environment check
		return weak_ptr<FMResultSet>();
	}
	if (!executeQueryParametersCheck(sizeof...(args), pStmt)) { // Parameters count check
		return weak_ptr<FMResultSet>();
	}
	bindObjects<1>(pStmt, std::forward<Args>(args)...);
	return executeQueryImpl(sql, statement, pStmt);
}

template<typename ...Args>
inline bool FMDatabase::executeUpdate(const string & sql, Args ...args)
{
	sqlite3_stmt *pStmt = 0;
	shared_ptr<FMStatement> statement;
	if (!executeQueryPrepareAndCheck(sql, pStmt, statement)) { // Sqlite environment check
		return nullptr;
	}
	if (!executeQueryParametersCheck(sizeof...(args), pStmt)) { // Parameters count check
		return nullptr;
	}
	bindObjects<1>(pStmt, std::forward<Args>(args)...);
	return executeUpdateImpl(sql,statement, pStmt);
}

template<int paramN, typename ...Args, typename T>
inline void FMDatabase::bindObjects(sqlite3_stmt * inStmt, const T & v, Args... args)
{
	bindObject(v, paramN, inStmt);
	bindObjects<paramN + 1>(inStmt, std::forward<Args>(args)...);
}

template<int paramN>
inline void FMDatabase::bindObjects(sqlite3_stmt * inStmt)
{}

/** convenience methods*/
template<typename... Args>
inline int FMDatabase::intForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->intForColumnIndex(0);
    rs->close();
    return n;
}

template<typename... Args>
inline long FMDatabase::longForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->longForColumnIndex(0);
    rs->close();
    return n;
}

template<typename... Args>
inline long long FMDatabase::longLongForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->longLongForColumnIndex(0);
    rs->close();
    return n;
}

template<typename... Args>
inline bool FMDatabase::boolForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->boolForColumnIndex(0);
    rs->close();
    return n;
}

template<typename... Args>
inline double FMDatabase::doubleForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->doubleForColumnIndex(0);
    rs->close();
    return n;
}

template<typename... Args>
inline String FMDatabase::stringForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->stringForColumnIndex(0);
    rs->close();
    return n;
}

template<typename... Args>
inline Data FMDatabase::dataForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->dataForColumnIndex(0);
    rs->close();
    return n;
}

template<typename... Args>
inline shared_ptr<Date> FMDatabase::dateForQuery(const string &sql, Args... args)
{
    auto rs = executeQuery(sql, std::forward<Args>(args)...).lock();
    if (!rs->next()) {
        return 0;
    }
    auto n = rs->dateForColumnIndex(0);
    rs->close();
    return n;
}

FMDB_END

#endif /* FMDatabase_hpp */
