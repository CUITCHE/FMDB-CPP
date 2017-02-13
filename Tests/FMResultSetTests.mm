//
//  FMResultSetTests.m
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/13.
//  Copyright © 2017年 CHE. All rights reserved.
//

#import "FMDBTempDBTests.h"
#import "FMDatabase.h"
#import "FMResultSet.h"
#include "Variant.hpp"
#include "Date.hpp"

#if FMDB_SQLITE_STANDALONE
#import <sqlite3/sqlite3.h>
#else
#import <sqlite3.h>
#endif

@interface FMResultSetTests : FMDBTempDBTests

@end

@implementation FMResultSetTests

+ (void)populateDatabase:(FMDatabase *)db
{
    db->executeUpdate("create table test (a text, b text, c integer, d double, e double)");

    db->beginTransaction();
    int i = 0;
    while (i++ < 20) {
        db->executeUpdate("insert into test (a, b, c, d, e) values (?, ?, ?, ?, ?)" ,
         "hi'",
         string("number").append(Variant(i).toString()),
         i,
         Date(),
         2.2f);
    }
    db->commit();
}

- (void)testNextWithError_WithoutError
{
    self.db->executeUpdate("CREATE TABLE testTable(key INTEGER PRIMARY KEY, value INTEGER)");
    self.db->executeUpdate("INSERT INTO testTable (key, value) VALUES (1, 2)");
    self.db->executeUpdate("INSERT INTO testTable (key, value) VALUES (2, 4)");

    FMResultSet *resultSet = self.db->executeQuery("SELECT * FROM testTable WHERE key=1");
    XCTAssert(resultSet);
//    NSError *error = nil;
    XCTAssertTrue(resultSet->nextWithError(nullptr));
//    XCTAssertNil(error);

    XCTAssertFalse(resultSet->nextWithError(0));
//    XCTAssertNil(error);

    resultSet->close();
}

- (void)testNextWithError_WithBusyError
{
    self.db->executeUpdate("CREATE TABLE testTable(key INTEGER PRIMARY KEY, value INTEGER)");
    self.db->executeUpdate("INSERT INTO testTable (key, value) VALUES (1, 2)");
    self.db->executeUpdate("INSERT INTO testTable (key, value) VALUES (2, 4)");

    FMResultSet *resultSet = self.db->executeQuery("SELECT * FROM testTable WHERE key=1");
    XCTAssert(resultSet);

    FMDatabase newDB(self.databasePath.UTF8String);
    newDB.open();

    newDB.beginTransaction();
//    NSError *error;
    XCTAssertFalse(resultSet->nextWithError(0));
    newDB.commit();


//    XCTAssertEqual(error.code, SQLITE_BUSY, @"SQLITE_BUSY should be the last error");
    resultSet->close();
}

- (void)testNextWithError_WithMisuseError
{
    self.db->executeUpdate("CREATE TABLE testTable(key INTEGER PRIMARY KEY, value INTEGER)");
    self.db->executeUpdate("INSERT INTO testTable (key, value) VALUES (1, 2)");
    self.db->executeUpdate("INSERT INTO testTable (key, value) VALUES (2, 4)");

    FMResultSet *resultSet = self.db->executeQuery("SELECT * FROM testTable WHERE key=9");
    XCTAssert(resultSet);
    XCTAssertFalse(resultSet->next());
//    NSError *error;
    XCTAssertFalse(resultSet->nextWithError(0));

//    XCTAssertEqual(error.code, SQLITE_MISUSE, @"SQLITE_MISUSE should be the last error");
}

@end
