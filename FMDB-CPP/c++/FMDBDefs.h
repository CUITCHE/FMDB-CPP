//
//  FMDBDefs.h
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/10.
//  Copyright © 2017年 CHE. All rights reserved.
//

#ifndef FMDBDefs_h
#define FMDBDefs_h

#ifndef __FMDB_NSPEC
#define __FMDB_NSPEC() FMDB_CPP
#endif

#ifndef FMDB_BEGIN
#define FMDB_BEGIN namespace __FMDB_NSPEC() {
#endif

#ifndef FMDB_END
#define FMDB_END }
#endif

#include <vector>
#include <memory>
#include <string>
#include <chrono>

using std::string;
using std::vector;
using std::shared_ptr;

FMDB_BEGIN

using Data = shared_ptr<vector<unsigned char>>;
using String = shared_ptr<string>;
using TimeInterval = std::chrono::duration<double>;
using Date = std::chrono::system_clock::time_point;

FMDB_END

#endif /* FMDBDefs_h */
