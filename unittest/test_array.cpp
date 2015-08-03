/*
 * test_array.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */
#include "test_array.h"

#include "configure.h"
#include <iostream>

using namespace std;


int main(int argc, char* argv[]) {
	if (!test_init(argc, argv)) {
		return -1;
	}

    test_chunk_array();
    test_lazy_array();

    return 0;
}

