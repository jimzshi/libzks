#include "configure.h"
#include "test.h"

#include <iostream>
#include <clocale>

using namespace std;

zks::simlog logger;

#ifdef ZKS_SINGLE_VS_SOLUTION_
int main(int argc, char* argv[])
{
    if (argc < 3) {
        cerr << argv[0] << " log.ini log-file" << endl;
        return 0;
    }
    logger.configure(argv[1]);
    logger.config.output.file_name = argv[2];
    logger.reset();

    //test_locale(argc, argv);
    //test_assign();
    //test_access();
    //test_insert();
    //test_format();
    //unicode_output();
    //test_append();
    //test_split();
    //test_join();
    //test_caseit();
    //test_stopwatch(logger);
    //test_simlog();
    //test_permutations();
    //test_mac();
    //test_hash1();
    //test_hash128();
    //test_hashcode<32>(false);
    //test_hashcode<64>(false);
    //test_hashcode<128>(false);
    //test_hashcode<256>(false);
    //test_hashcode<32>(true);
    //test_hashcode<64>(true);
    //test_hashcode<128>(true);
    for(int i=0; i<10000; ++i) test_hashcode<64>(true);
    //test_hashcode<32, 256, char>(true);
    //test_hashcode<256, 128, char>(true);   //failed!
    //test_hashcode_128(false);
    //test_hashcode_256(true);
    //test_bit();
    //test_chunk_array();
    //test_code();
    //test_hashvector(argc, argv);
    //test_hash_table(argc, argv);
    //test_bitvector();
    //test_bitvector_perf();
    //test_rand();
    //test_foreach2();
    return 0;
}
#endif
