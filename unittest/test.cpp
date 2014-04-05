//#include "test_locale.h"
#include "test.h"
#include <iostream>

using namespace std;

zks::simlog logger;

int main(int argc, char* argv[]) {
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
	size_t cardinals[5] = { 3, 4, 5, 4, 3 };
	zks::Permutations<5> perm{ cardinals };
	//for (size_t i = 0; i < perm.size(); ++i, perm.next()) {
	//	ZKS_ERROR(logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", i, perm.size(), 
	//		perm.index(0), perm.index(1), perm.index(2), perm.index(3), perm.index(4));
	//}
	perm.begin_at(13);
	ZKS_ERROR(logger, "perm", "%5d/%5d: {%d, %d, %d, %d, %d}", 13, perm.size(),
		perm.index(0), perm.index(1), perm.index(2), perm.index(3), perm.index(4));
	return 0;
}
