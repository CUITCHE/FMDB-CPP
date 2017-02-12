// FMDB-CPP-MSVS.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include "FMDB.h"
#include "Variant.hpp"
#define _STR(x) #x
using namespace __FMDB_NSPEC();
int main()
{
	std::cout << "hello world!" << std::endl;
	FMDatabase db("");
	db.executeQuery("nn");
    return 0;
}

