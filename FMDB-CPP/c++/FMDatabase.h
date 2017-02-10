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

using std::unordered_map;
using std::unordered_set;
using std::unique_ptr;

typedef struct sqlite3 sqlite3;

FMDB_BEGIN

extern const char *const FMDB_CPP_DATE_FORMAT;

class FMStatement;
class FMResultSet;

using FMDate = std::chrono::time_point<std::chrono::system_clock>;

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

    void setMaxBusyRetryTimeInterval(double timeoutInSeconds);
    double maxBusyRetryTimeInterval() const { return _maxBusyRetryTimeInterval.count(); };
private:
    const char *sqlitePath() const;
    friend int FMDBDatabaseBusyHandler(void *f, int count);

    FMStatement *cachedStatementForQuery(const string &query);
    void setCachedStatement(FMStatement *statement, const string &query);

    void warnInUse() const;
    bool databaseExists() const;
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
    Date _startBusyRetryTime;
    StatemenCacheType _cachedStatements;
    unique_ptr<unordered_set<FMResultSet *>> _openResultSets;
    unique_ptr<unordered_set<FMStatement *>> _openFunctions;
    const string _databasePath;
};

FMDB_END

#endif /* FMDatabase_hpp */
