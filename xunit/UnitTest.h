#ifndef LIBZKS_UNITTEST_H_
#define LIBZKS_UNITTEST_H_

#include "libzks.h"
#include "unittest/test_hash.h"
#include "cxxtest/TestSuite.h"

zks::simlog g_logger;

class MyTestSuite1 : public CxxTest::TestSuite
{
public:
	void testAddition(void)
	{
		//char* argv[] = { "", R"(c:\local\etc\simlog.ini)", R"(c:\local\log\hash_table.txt)", R"(c:\local\doc\QlikViewError.Log)", "7"};
        const char* argv[] = { "", "~/local/etc/simlog.ini", "~/local/log/hash_table.txt", "~/local/doc/QlikViewError.Log", "7"};
		g_logger.configure(argv[1]);
		g_logger.config.output.file_name = argv[2];
		g_logger.reset();
		test_hashvector(5, argv);
		TS_ASSERT(1 + 1 == 2);
		TS_ASSERT_EQUALS(1 + 1, 2);
	}
};

#endif
