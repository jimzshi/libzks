#include "configure.h"
#include "test.h"
#include "random.h"

#include <iostream>
#include <clocale>

using namespace std;

zks::simlog g_logger;
zks::simconf g_settings;
zks::RangedRNGen<int> g_test_rng(1, 1000000);

bool test_init(int argc, char* const argv[]) {
	if (argc < 3) {
		cerr << argv[0] << " settings.ini log-file" << endl;
		return false;
	}
	if (g_settings.parse(argv[1]) < 0) {
		cerr << " invalid settings.ini" << endl;
		return false;
	}
	g_logger.configure(argv[1]);
	g_logger.config.output.file_name = argv[2];
	g_logger.reset();

	zks::randomize();

	return true;
}
