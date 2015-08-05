/*
 * test_distributor.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */

#include "test_distributor.h"
#include "configure.h"
#include <iostream>

using namespace std;


int main(int argc, const char* argv[]) {
	if (!test_init(argc, argv)) {
		return -1;
	}
#ifndef ZKS_OS_WINDOWS_
    test_dist();
    test_dist2();
#endif

    test_foreach();
    test_foreach2();

    return 0;
}
