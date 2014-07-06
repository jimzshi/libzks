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


#ifdef ZKS_OS_GNULINUX_
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << argv[0] << " log.ini log-file" << endl;
        return 0;
    }
    logger.configure(argv[1]);
    logger.config.output.file_name = argv[2];
    logger.reset();

    test_permutations();

    return 0;
}
#endif

