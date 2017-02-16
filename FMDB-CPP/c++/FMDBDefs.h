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
#include <cassert>

using std::string;
using std::vector;
using std::shared_ptr;
using std::weak_ptr;

FMDB_BEGIN

using namespace std::chrono;

using Data = shared_ptr<vector<unsigned char>>;
using String = shared_ptr<string>;
using TimeInterval = std::chrono::duration<double>;

#ifdef DEBUG

# define ____fn____ __PRETTY_FUNCTION__

# define _assert(cond, desc, ...) \
    do {\
        if (!(cond)) {\
            fprintf(stderr, "*** Assertion failure in [%s], %s:%d => ('%s')", ____fn____, __FILE__, __LINE__, #cond);\
            if (*desc) {\
                putchar(','), putchar(' ');\
                fprintf(stderr, desc, ##__VA_ARGS__);\
            }\
            putchar('\n');\
            abort();\
        }\
    } while (0)
# define parameterAssert(cond) _assert((cond), "Invalid parameter not satisfying: %s", #cond)
#else
# define _assert(cond, desc, ...)
# define parameterAssert(c)
#endif // DEBUG

#ifndef __PL64__
# ifdef WIN32
#  ifdef _WIN64
#   define __PL64__ 1
#  else
#   define __PL64__ 0
#  endif
# endif

# ifdef __clang__
#  define __PL64__ __LP64__
# endif
#endif // __PL64__

#ifndef DO_NOT_RELEASE
#define DO_NOT_RELEASE(t) t
#endif // DON_NOT_RELEASE

FMDB_END

#endif /* FMDBDefs_h */
