/*
 * test_hash.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */
#include "test_hash.h"

#include "configure.h"
#include <iostream>

using namespace std;


#ifndef ZKS_SINGLE_VS_SOLUTION_
int main(int argc, const char* argv[]) {
	if (!test_init(argc, argv)) {
		return -1;
	}

    //test_hash();
    //test_hash128();
    //test_hashcode(false);
    //test_hashcode_128(false);
    test_hashvector(argc, argv);

    return 0;
}
#endif

