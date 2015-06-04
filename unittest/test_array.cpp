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


#ifndef ZKS_SINGLE_VS_SOLUTION_
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << argv[0] << " log.ini log-file" << endl;
        return 0;
    }
    logger.configure(argv[1]);
    logger.config.output.file_name = argv[2];
    logger.reset();

    test_chunk_array();
    test_lazy_array();

    return 0;
}
#endif

