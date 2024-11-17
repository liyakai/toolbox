#include "unit_test_frame/unittest.h"
#include "tools/md5.h"
#include <thread>

FIXTURE_BEGIN(MD5Test)

CASE(TestMD5)
{
    ToolBox::MD5 md5("hello");
    fprintf(stderr, "md5 32: %s\n", md5.toStr().c_str());
}

FIXTURE_END(TestMD5)