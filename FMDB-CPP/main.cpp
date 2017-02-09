//
//  main.cpp
//  FMDB-CPP
//
//  Created by hejunqiu on 2017/2/9.
//  Copyright © 2017年 CHE. All rights reserved.
//

#include <iostream>
#include "FMDB.h"
#include <thread>

int main(int argc, const char * argv[]) {
    auto start = std::chrono::system_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::chrono::duration<double> duration = std::chrono::system_clock::now() - start;
    std::cout << duration.count() << std::endl;
    return 0;
}
