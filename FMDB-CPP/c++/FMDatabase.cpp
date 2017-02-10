//
//  FMDatabase.cpp
//  fmdb
//
//  Created by hejunqiu on 2017/2/9.
//
//

#include "FMDatabase.h"
#include "FMStatement.hpp"
#include "FMResultSet.h"
#include <cassert>
#include <sqlite3.h>

using namespace std;

FMDB_BEGIN

FMDatabase::FMDatabase(const string &path)
:_logsErrors(true)
,_crashOnErrors(false)
,_traceExecution(0)
,_checkedOut(0)
,_shouldCacheStatements(0)
,_isExecutingStatement(0)
,_inTransaction(0)
,_cachedStatements(new (nothrow) decltype(_cachedStatements)::element_type())
,_openResultSets(new (nothrow) decltype(_openResultSets)::element_type)
,_openFunctions(new (nothrow) decltype(_openFunctions)::element_type)
,_databasePath(path)
{
    assert(sqlite3_threadsafe());
}

FMDatabase::~FMDatabase()
{
    close();
}

const char *FMDatabase::sqlitePath() const
{
    if (_databasePath.length() == 0) {
        return "";
    }
    return _databasePath.c_str();
}

bool FMDatabase::open()
{
    if (_db) {
        return true;
    }
    int err = sqlite3_open(sqlitePath(), (sqlite3 **)&_db);
    if (err != SQLITE_OK) {
        fprintf(stderr, "open error:%d\n", err);
        return false;
    }
    if (_maxBusyRetryTimeInterval.count() > 0) {
        setMaxBusyRetryTimeInterval(_maxBusyRetryTimeInterval.count());
    }
    return true;
}

bool FMDatabase::openWithFlags(int flags)
{
    return openWithFlags(flags, nullptr);
}

bool FMDatabase::openWithFlags(int flags, const string &vfs)
{
#if SQLITE_VERSION_NUMBER >= 3005000
    if (_db) {
        return true;
    }

    int err = sqlite3_open_v2(sqlitePath(), (sqlite3**)&_db, flags, vfs.c_str());
    if(err != SQLITE_OK) {
        fprintf(stderr, "error opening!: %d\n", err);
        return false;
    }

    if (_maxBusyRetryTimeInterval > TimeInterval(0)) {
        // set the handler
        setMaxBusyRetryTimeInterval(_maxBusyRetryTimeInterval.count());
    }

    return true;
#else
    fprintf(stderr, "openWithFlags requires SQLite 3.5\n");
    return false;
#endif
}

bool FMDatabase::close()
{
    clearCachedStatements();
    closeOpenResultSets();

    if (!_db) {
        return true;
    }
    int  rc;
    bool retry;
    bool triedFinalizingOpenStatements = false;

    do {
        retry   = false;
        rc      = sqlite3_close(_db);
        if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc) {
            if (!triedFinalizingOpenStatements) {
                triedFinalizingOpenStatements = true;
                sqlite3_stmt *pStmt;
                while ((pStmt = sqlite3_next_stmt(_db, nullptr)) !=0) {
                    fprintf(stderr, "Closing leaked statement\n");
                    sqlite3_finalize(pStmt);
                    retry = true;
                }
            }
        }
        else if (SQLITE_OK != rc) {
            fprintf(stderr, "error closing!: %d", rc);
        }
    }
    while (retry);
    _db = nullptr;
    return true;
}

#pragma mark Busy handler routines

// NOTE: appledoc seems to choke on this function for some reason;
//       so when generating documentation, you might want to ignore the
//       .m files so that it only documents the public interfaces outlined
//       in the .h files.
//
//       This is a known appledoc bug that it has problems with C functions
//       within a class implementation, but for some reason, only this
//       C function causes problems; the rest don't. Anyway, ignoring the .m
//       files with appledoc will prevent this problem from occurring.

int FMDBDatabaseBusyHandler(void *f, int count)
{
    FMDatabase *self = (FMDatabase *)f;

    if (count == 0) {
        self->_startBusyRetryTime = std::chrono::system_clock::now();
        return 1;
    }

    TimeInterval delta = std::chrono::system_clock::now() - (self->_startBusyRetryTime);

    if (delta < self->_maxBusyRetryTimeInterval) {
#ifdef _MSC_VER
		int requestedSleepInMillseconds = 50;
#else
		int requestedSleepInMillseconds = (int)arc4random_uniform(50) + 50;
#endif
        
        int actualSleepInMilliseconds = sqlite3_sleep(requestedSleepInMillseconds);
        if (actualSleepInMilliseconds != requestedSleepInMillseconds) {
            fprintf(stderr, "WARNING: Requested sleep of %i milliseconds, but SQLite returned %i. Maybe SQLite wasn't built with HAVE_USLEEP=1?\n", requestedSleepInMillseconds, actualSleepInMilliseconds);
        }
        return 1;
    }

    return 0;
}

void FMDatabase::setMaxBusyRetryTimeInterval(double timeout)
{
    _maxBusyRetryTimeInterval = decltype(_maxBusyRetryTimeInterval)(timeout);
    if (!_db) {
        return;
    }

    if (timeout > 0) {
        sqlite3_busy_handler(_db, &FMDBDatabaseBusyHandler, this);
    } else {
        sqlite3_busy_handler(_db, nullptr, nullptr);
    }
}

bool FMDatabase::hasOpenResultSets()
{
    return _openResultSets->size() > 0;
}

void FMDatabase::closeOpenResultSets()
{
    for (auto &tmp : *_openResultSets) {
        tmp->setParentDB(nullptr);
        tmp->close();
        delete tmp;
    }
    _openResultSets->clear();
}

void FMDatabase::resultSetDidClose(FMResultSet *resultSet)
{
    _openResultSets->erase(resultSet);
}

#pragma mark Cached statements
void FMDatabase::clearCachedStatements()
{
    for (auto &pair : *_cachedStatements) {
        for (auto statement : pair.second) {
            statement->close();
            delete statement;
        }
    }
    _cachedStatements->clear();
}

FMStatement *FMDatabase::cachedStatementForQuery(const string &query)
{
    auto &statements = _cachedStatements->at(query);
    FMStatement *st = nullptr;
    for (auto tmp : statements) {
        if (!tmp->inUse()) {
            st = tmp;
            break;
        }
    }
    return st;
}

void FMDatabase::setCachedStatement(FMStatement *statement, const string &query)
{
    statement->setQueryString(query);

    auto iter = _cachedStatements->find(query);
    if (iter == _cachedStatements->end()) {
        unordered_set<FMStatement *> statements;
        statements.insert(statement);
        _cachedStatements->emplace(query, statements);
    } else {
        auto &statements = iter->second;
        statements.insert(statement);
    }
}

#pragma mark Key routines
bool FMDatabase::reKey(const string &key)
{
    vector<char> dd(key.begin(), key.end());
    return reKey(dd);
}

bool FMDatabase::reKey(const vector<char> &key)
{
#ifdef SQLITE_HAS_CODEC
    if (key.size() == 0) {
        return false;
    }

    int rc = sqlite3_rekey(_db, (const void *)key.data(), (int)key.size());

    if (rc != SQLITE_OK) {
        fprintf(stderr, "error on rekey: %d\n", rc);
        fprintf(stderr, "%s", lastErrorMessage().c_str());
    }

    return (rc == SQLITE_OK);
#else
#pragma unused(key)
    return false;
#endif
}

bool FMDatabase::setKey(const string &key)
{
    vector<char> dd(key.begin(), key.end());
    return setKey(dd);
}

bool FMDatabase::setKey(const vector<char> &key)
{
#ifdef SQLITE_HAS_CODEC
    if (key.size() == 0) {
        return false;
    }

    int rc = sqlite3_key(_db, (const void *)key.data(), (int)key.size());

    return (rc == SQLITE_OK);
#else
#pragma unused(key)
    return false;
#endif
}

#pragma mark State of database
bool FMDatabase::isGoodConnection() const
{
    if (!_db) {
        return false;
    }
    FMResultSet *rs = nullptr; // TODO:[self executeQuery:@"select name from sqlite_master where type='table'"];
    if (rs) {
        rs->close();
        return true;
    }
    return false;
}

void FMDatabase::warnInUse() const
{
    fprintf(stderr, "The FMDatabase %p is currently in use.\n", this);

#ifndef NS_BLOCK_ASSERTIONS
    if (_crashOnErrors) {
        abort();
    }
#endif
}

bool FMDatabase::databaseExists() const
{
    if (!_db) {
        fprintf(stderr, "The FMDatabase %p is not open.\n", this);
#ifndef NS_BLOCK_ASSERTIONS
        if (_crashOnErrors) {
            abort();
        }
#endif
        return false;
    }
    return true;
}

#pragma mark Error routines
string FMDatabase::lastErrorMessage() const
{
    return string(sqlite3_errmsg(_db));
}

bool FMDatabase::hadError() const
{
    int lastErrCode = lastErrorCode();
    return (lastErrCode > SQLITE_OK && lastErrCode < SQLITE_ROW);
}

int FMDatabase::lastErrorCode() const
{
    return sqlite3_errcode(_db);
}

int FMDatabase::lastExtendedErrorCode() const
{
    return sqlite3_extended_errcode(_db);
}

#pragma mark Update information routines
long long FMDatabase::lastInsertRowId()
{
    if (_isExecutingStatement) {
        warnInUse();
        return -1;
    }
    _isExecutingStatement = true;
    sqlite_int64 ret = sqlite3_last_insert_rowid(_db);
    _isExecutingStatement = false;
    return ret;
}

int FMDatabase::changes()
{
    if (_isExecutingStatement) {
        warnInUse();
        return -1;
    }

    _isExecutingStatement = true;
    int ret = sqlite3_changes(_db);
    _isExecutingStatement = false;

    return ret;
}

#pragma mark SQL manipulation
template<typename T>
void bindObject(T obj, int toColumn, sqlite3_stmt *stmt) {
    static_assert(std::is_pointer<T>::value == false, "Don't support pointer");
    sqlite3_bind_int(stmt, toColumn, obj);
}

template <>
void bindObject<sqlite3_int64>(sqlite3_int64 obj, int toColumn, sqlite3_stmt *stmt) {
    sqlite3_bind_int64(stmt, toColumn, obj);
}

template <>
void bindObject<double>(double obj, int toColumn, sqlite3_stmt *stmt) {
    sqlite3_bind_double(stmt, toColumn, obj);
}

template <>
void bindObject<float>(float obj, int toColumn, sqlite3_stmt *stmt) {
    sqlite3_bind_double(stmt, toColumn, obj);
}

template <>
void bindObject<const string&>(const string &obj, int toColumn, sqlite3_stmt *stmt) {
    sqlite3_bind_text(stmt, toColumn, obj.c_str(), -1, SQLITE_STATIC);
}

template <>
void bindObject<const vector<char>&>(const vector<char> &obj, int toColumn, sqlite3_stmt *stmt) {
    auto bytes = obj.data();
    int length = (int)obj.size();
    if (!bytes) {
        bytes = "";
        length = 0;
    }
    sqlite3_bind_blob(stmt, toColumn, bytes, length, SQLITE_STATIC);
}

const char *const FMDB_CPP_DATE_FORMAT = "%a %b %d %Y %H:%M:%S %Z";

template <>
void bindObject<const FMDate&> (const FMDate &obj, int toColumn, sqlite3_stmt *stmt) {
    time_t time = std::chrono::system_clock::to_time_t(obj);
    char str[64];
    struct tm tm;
#ifdef _MSC_VER
	localtime_s(&tm, &time);
#else
	localtime_r(&time, &tm);
#endif
    
    strftime(str, sizeof(str), FMDB_CPP_DATE_FORMAT, &tm);
    sqlite3_bind_text(stmt, toColumn, str, -1, SQLITE_STATIC);
}

FMDB_END
