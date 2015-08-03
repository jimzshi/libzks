/*
 * test_bit.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */
#include "test_bit.h"

#include "configure.h"
#include <iostream>

using namespace std;


#ifndef ZKS_SINGLE_VS_SOLUTION_
int main(int argc, char* argv[]) {
	if (!test_init(argc, argv)) {
		return -1;
	}

    test_bit();
    test_bitvector();
    test_bitvector_perf();

    return 0;
}
#endif

