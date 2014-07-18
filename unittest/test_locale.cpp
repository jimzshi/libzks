/*
 * test_locale.cpp
 *
 *  Created on: Jul 6, 2014
 *      Author: zks
 */
#include "test_locale.h"

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

    test_locale(argc, argv);
    test_assign();
    test_access();
    test_insert();
    test_format();
    unicode_output();
    test_append();
    test_split();
    test_join();
    test_caseit();

    return 0;
}
#endif

