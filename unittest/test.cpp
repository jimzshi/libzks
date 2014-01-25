//#include "test_locale.h"
#include "test.h"
#include <iostream>

using namespace std;

zks::simlog logger;

int main(int argc, char* argv[]) {
	//logger.configure((R"(C:\github\jimzshi\libzks\src\simlog.ini)"));
	logger.configure((R"(/home/zikang/github/jimzshi/libzks/src/simlog.ini)"));
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
	test_simlog();
	return 0;
}
