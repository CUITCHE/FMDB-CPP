//
//  main.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/9.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include <iostream>
#include "FMDB.h"
#include "Date.hpp"

using namespace std;
using namespace __FMDB_NSPEC();

int main(int argc, const char * argv[]) {
    cout << sizeof(unique_ptr<std::unordered_map<string, Variant>>) << endl;
    return 0;
}
