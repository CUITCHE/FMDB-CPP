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
#include "Date.hpp"
/*#include "Variant.hpp"*/
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
    _assert(sqlite3_threadsafe(), "On no thread safe. sqlite3 might not work well.");
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
        setMaxBusyRetryTimeInterval(_maxBusyRetryTimeInterval);
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
        setMaxBusyRetryTimeInterval(_maxBusyRetryTimeInterval);
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
                sqlite3_stmt *pStmt = 0;
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
        self->_startBusyRetryTime = Date().timeIntervalSinceReferenceDate();
        return 1;
    }

    TimeInterval delta = Date().timeIntervalSinceReferenceDate() - (self->_startBusyRetryTime);

    if (delta < self->_maxBusyRetryTimeInterval) {
#ifdef WIN32
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

void FMDatabase::setMaxBusyRetryTimeInterval(TimeInterval timeout)
{
    _maxBusyRetryTimeInterval = timeout;
    if (!_db) {
        return;
    }

    if (timeout > TimeInterval(0)) {
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

bool FMDatabase::beginTransaction()
{
	bool b = executeUpdate("begin exclusive transaction");
	if (b) {
		_inTransaction = true;
	}
	return b;
}

bool FMDatabase::beginDeferredTransaction()
{
	bool b = executeUpdate("begin deferred transaction");
	if (b) {
		_inTransaction = true;
	}
	return b;
}

bool FMDatabase::commit()
{
	bool b = executeUpdate("commit transaction");
	if (b) {
		_inTransaction = false;
	}
	return b;
}

bool FMDatabase::rollback()
{
	bool b = executeUpdate("rollback transaction");
	if (b) {
		_inTransaction = false;
	}
	return b;
}

bool FMDatabase::inTransaction()
{
	return _inTransaction;
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

bool FMDatabase::interrupt()
{
	if (_db) {
		sqlite3_interrupt(sqliteHandle());
		return true;
	}
	return false;
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
void FMDatabase::bindObject(const Variant & obj, int toColumn, sqlite3_stmt * inStmt)
{
	if (!obj || obj == Variant::null) {
		sqlite3_bind_null(inStmt, toColumn);
	} else if (obj.isTypeOf(Variant::Type::DATA)) {
        auto &data = obj.toVariantData();
        const void *bytes = data.data();
        size_t length = data.size();
        if (!bytes) {
            bytes = "";
            length = 0;
        }
        sqlite3_bind_blob(inStmt, toColumn, bytes, (int)length, SQLITE_STATIC);
	} else if (obj.isTypeOf(Variant::Type::DATE)) {
        auto &date = obj.toDate();
#ifdef FMDB_CONCERN_MEMORY
		sqlite3_bind_double(inStmt, toColumn, date.timeIntervalSince1970().count());
#else
        string dateString = Date::stringFromDate(date);
        sqlite3_bind_text(inStmt, toColumn, dateString.c_str(), (int)dateString.size(), SQLITE_STATIC);
#endif
	} else if (obj.isTypeOf(Variant::Type::STRING)){
		auto &str = const_cast<Variant &>(obj).toString();
		sqlite3_bind_text(inStmt, toColumn, str.c_str(), (int)str.size(), SQLITE_STATIC);
	} else {
#define _STR(x) #x
		switch (obj.getType())
		{
		case FMDB_CPP::Variant::Type::BOOLEAN: sqlite3_bind_int(inStmt, toColumn, obj.toBool());
			break;
		case FMDB_CPP::Variant::Type::CHAR: sqlite3_bind_int(inStmt, toColumn, obj.toChar());
			break;
		case FMDB_CPP::Variant::Type::BYTE: sqlite3_bind_int(inStmt, toColumn, obj.toByte());
			break;
		case FMDB_CPP::Variant::Type::INTEGER: sqlite3_bind_int(inStmt, toColumn, obj.toInt());
			break;
		case FMDB_CPP::Variant::Type::UINTEGER: sqlite3_bind_int(inStmt, toColumn, obj.toUInt());
			break;
		case FMDB_CPP::Variant::Type::FLOAT: sqlite3_bind_double(inStmt, toColumn, obj.toFloat());
			break;
		case FMDB_CPP::Variant::Type::DOUBLE: sqlite3_bind_double(inStmt, toColumn, obj.toDouble());
			break;
		case FMDB_CPP::Variant::Type::LONGLONG: sqlite3_bind_int64(inStmt, toColumn, obj.toLongLong());
			break;
		case FMDB_CPP::Variant::Type::ULONGLONG: sqlite3_bind_int64(inStmt, toColumn, obj.toULongLong());
			break;
		case FMDB_CPP::Variant::Type::VARIANTVECTOR:
		case FMDB_CPP::Variant::Type::VARIANTMAP:
		case FMDB_CPP::Variant::Type::VARIANTMAPINTKEY: 
			_assert(0, "Don't support (%s, %s, %s) to write to sqlite.\n", _STR(VariantVector), _STR(VariantMap), _STR(VariantMapIntKey));
			break;
 		default:
 			break;
		}
#undef _STR
	}
}

bool FMDatabase::executeQueryPrepareAndCheck(const string &sql, sqlite3_stmt *&pStmt, FMStatement *&statement)
{
	if (!databaseExists()) {
		return false;
	}
	if (_isExecutingStatement) {
		warnInUse();
		return false;
	}
	_isExecutingStatement = true;

	if (_traceExecution) {
		fprintf(stdout, "<%p> executeQuery:%s\n", this, sql.c_str());
	}
	if (_shouldCacheStatements) {
		statement = cachedStatementForQuery(sql);
		if (statement) {
			pStmt = statement->getStatement();
			statement->reset();
		}
	}
	if (!pStmt) {
		int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &pStmt, 0);
		if (rc != SQLITE_OK) {
			if (_logsErrors) {
				fprintf(stdout, "DB Error:%d, \"%s\"\n", lastErrorCode(), lastErrorMessage().c_str());
				fprintf(stdout, "DB Query:%s\n", sql.c_str());
				fprintf(stdout, "DB Path:%s\n", _databasePath.c_str());
			}
			if (_crashOnErrors) {
				fprintf(stderr, "DB Error:%d, \"%s\"\n", lastErrorCode(), lastErrorMessage().c_str());
				abort();
			}
			sqlite3_finalize(pStmt);
			_isExecutingStatement = false;
			return false;
		}
	}
	return true;
}

bool FMDatabase::executeQueryParametersCheck(int inputParametersCount, sqlite3_stmt * pStmt)
{
	int bindCount = sqlite3_bind_parameter_count(pStmt);
	if (inputParametersCount != bindCount) {
		fprintf(stderr, "Error: the bind count(%d) is not correct for the # of variables (executeQuery:%d)\n", inputParametersCount, bindCount);
		sqlite3_finalize(pStmt);
		_isExecutingStatement = false;
		return false;
	}
	return true;
}

FMResultSet * FMDatabase::executeQueryImpl(const string & sql, FMStatement *statement, sqlite3_stmt *pStmt)
{
	if (!statement) {
		statement = new FMStatement();
		statement->setStatement(pStmt);

		if (_shouldCacheStatements) {
			setCachedStatement(statement, sql);
		}
	}

	// the statement gets closed in rs's dealloc or [rs close];
	FMResultSet *rs = FMResultSet::resultSet(statement, this);
	rs->setQuery(sql);

	_openResultSets->insert(rs);
	statement->setUseCount(statement->getUseCount() + 1);

	_isExecutingStatement = false;

	return rs;
}

bool FMDatabase::executeUpdateImpl(const string & sql, FMStatement * statement, sqlite3_stmt * pStmt)
{
	int rc = sqlite3_step(pStmt);
	if (rc == SQLITE_DONE) {
		//
	} else if (rc == SQLITE_INTERRUPT) {
		if (_logsErrors) {
			fprintf(stderr, "Error calling sqlite3_step. Query was interrupted (%d: %s) SQLITE_INTERRUPT", rc, sqlite3_errmsg(_db));
			fprintf(stderr, "DB Query: %s", sql.c_str());
		}
	} else if (rc == SQLITE_ROW) {
		if (_logsErrors) {
			fprintf(stderr, "A executeUpdate is being called with a query string '%s'", sql.c_str());
			fprintf(stderr, "DB Query: %s", sql.c_str());
		}
	} else {
		if (rc == SQLITE_ERROR) {
			if (_logsErrors) {
				fprintf(stderr, "Error calling sqlite3_step(%d: %s) SQLITE_ERROR", rc, sqlite3_errmsg(_db));
				fprintf(stderr, "DB Query: %s", sql.c_str());
			}
		} else if (rc == SQLITE_MISUSE) {
			if (_logsErrors) {
				fprintf(stderr, "Error calling sqlite3_step(%d: %s) SQLITE_MISUSE", rc, sqlite3_errmsg(_db));
				fprintf(stderr, "DB Query: %s", sql.c_str());
			}
		} else {
			if (_logsErrors) {
				fprintf(stderr, "Unknown error calling sqlite3_step(%d: %s) eu", rc, sqlite3_errmsg(_db));
				fprintf(stderr, "DB Query: %s", sql.c_str());
			}
		}
	}
	if (_shouldCacheStatements && !statement) {
		statement = new FMStatement;
		statement->setStatement(pStmt);
		setCachedStatement(statement, sql);
	}
	int closeErrorCode = 0;
	if (statement) {
		statement->setUseCount(statement->getUseCount() + 1);
		closeErrorCode = sqlite3_reset(pStmt);
	} else {
		closeErrorCode = sqlite3_finalize(pStmt);
	}
	if (closeErrorCode != SQLITE_OK) {
		if (_logsErrors) {
			fprintf(stderr, "Unknown error finalizing or resetting statement(%d: %s)", closeErrorCode, sqlite3_errmsg(_db));
			fprintf(stderr, "DB Query: %s", sql.c_str());
		}
	}
	_isExecutingStatement = false;
	return rc == SQLITE_DONE || rc == SQLITE_OK;
	return false;
}

FMDB_END
