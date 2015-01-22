#ifndef LIBZKS_UNITTEST_H_
#define LIBZKS_UNITTEST_H_

#include "libzks.h"
#include "cxxtest/TestSuite.h"

class MyTestSuite1 : public CxxTest::TestSuite
{
public:
	void testAddition(void)
	{
		TS_ASSERT(1 + 1 == 1);
		TS_ASSERT_EQUALS(1 + 1, 2);
	}
};

#endif
