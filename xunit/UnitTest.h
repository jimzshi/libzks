#ifndef LIBZKS_UNITTEST_H_
#define LIBZKS_UNITTEST_H_

#include "libzks.h"
#include "unittest\test_hash.h"
#include "cxxtest/TestSuite.h"

zks::simlog logger;

class MyTestSuite1 : public CxxTest::TestSuite
{
public:
	void testAddition(void)
	{
		char* argv[] = { "", R"(c:\local\etc\simlog.ini)", R"(c:\local\log\hash_table.txt)", R"(c:\local\doc\QlikViewError.Log)", "7"};
		logger.configure(argv[1]);
		logger.config.output.file_name = argv[2];
		logger.reset();
		test_hash_table(5, argv);
		TS_ASSERT(1 + 1 == 2);
		TS_ASSERT_EQUALS(1 + 1, 2);
	}
};

#endif
