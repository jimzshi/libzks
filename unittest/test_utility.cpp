/*
 * test_utility.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */
#include "test_utility.h"

#include "configure.h"
#include <iostream>

using namespace std;


#ifndef ZKS_SINGLE_VS_SOLUTION_
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << argv[0] << " log.ini log-file" << endl;
        return 0;
    }
    g_logger.configure(argv[1]);
    g_logger.config.output.file_name = argv[2];
    g_logger.reset();

    test_permutations();

    return 0;
}
#endif

