#include "unit_test_frame/unittest.h"

int32_t main()
{
	UnitTestMgr.RunAllFixture();
	UnitTestMgr.AnyKeyToQuit();
	return 0;
}